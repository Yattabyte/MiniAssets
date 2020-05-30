#include "assetManager.hpp"
#include <algorithm>
#include <chrono>
#include <thread>

///////////////////////////////////////////////////////////////////////////
/// Use our shared namespace mini
using namespace mini;

///////////////////////////////////////////////////////////////////////////
/// shareAsset
///////////////////////////////////////////////////////////////////////////

Shared_Asset AssetManager::shareAsset(
    const std::string& assetType, const std::string& filename,
    const std::function<Shared_Asset(void)>& constructor,
    const bool& threaded) {
    // Find out if the asset already exists
    std::shared_lock<std::shared_mutex> asset_read_guard(m_mutexAssetMap);
    for (const auto& asset : m_assetMap[assetType])
        if (asset->getFileName() == filename) {
            asset_read_guard.unlock();
            asset_read_guard.release();
            // Check if we need to wait for initialization
            if (!threaded)
                // Stay here until asset finalizes
                while (!asset->ready())
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return asset;
        }
    asset_read_guard.unlock();
    asset_read_guard.release();

    // Create the asset
    std::unique_lock<std::shared_mutex> asset_write_guard(m_mutexAssetMap);
    const auto& asset = constructor();
    m_assetMap[assetType].push_back(asset);
    asset_write_guard.unlock();
    asset_write_guard.release();

    // Initialize now or later, depending if we are threading this order or not
    if (threaded) {
        std::unique_lock<std::shared_mutex> worker_write_guard(
            m_mutexWorkorders);
        m_workOrders.emplace_back(std::bind(&Asset::initialize, asset));
    } else
        asset->initialize();
    return asset;
}

///////////////////////////////////////////////////////////////////////////
/// beginWorkOrder
///////////////////////////////////////////////////////////////////////////

void AssetManager::beginWorkOrder() {
    // Start reading work orders
    std::unique_lock<std::shared_mutex> writeGuard(m_mutexWorkorders);
    if (!m_workOrders.empty()) {
        // Remove front of queue
        const Asset_Work_Order workOrder = m_workOrders.front();
        m_workOrders.pop_front();
        writeGuard.unlock();
        writeGuard.release();

        // Initialize asset
        workOrder();
    }
}

///////////////////////////////////////////////////////////////////////////
/// submitNotifyee
///////////////////////////////////////////////////////////////////////////

void AssetManager::submitNotifyee(
    const std::pair<std::shared_ptr<bool>, std::function<void()>>& callBack) {
    std::unique_lock<std::shared_mutex> writeGuard(m_mutexNofications);
    m_notifyees.push_back(callBack);
}

///////////////////////////////////////////////////////////////////////////
/// notifyObservers
///////////////////////////////////////////////////////////////////////////

void AssetManager::notifyObservers() {
    std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>>
        copyNotifyees;
    {
        std::unique_lock<std::shared_mutex> writeGuard(m_mutexNofications);
        copyNotifyees = m_notifyees;
        m_notifyees.clear();
    }

    for (const auto& pair : copyNotifyees)
        if (pair.first)
            pair.second();
}

///////////////////////////////////////////////////////////////////////////
/// readyToUse
///////////////////////////////////////////////////////////////////////////

bool AssetManager::readyToUse() {
    std::shared_lock<std::shared_mutex> readGuard(m_mutexWorkorders);
    if (!m_workOrders.empty())
        return false;

    readGuard = std::shared_lock<std::shared_mutex>(m_mutexAssetMap);
    return std::all_of(
        m_assetMap.begin(), m_assetMap.end(), [](const auto& assetCategory) {
            return std::all_of(
                assetCategory.second.cbegin(), assetCategory.second.cend(),
                [](const auto& asset) { return asset->ready(); });
        });
}