#pragma once
#ifndef MINI_ASSETMANAGER_HPP
#define MINI_ASSETMANAGER_HPP

#include "asset.hpp"
#include <deque>
#include <map>
#include <shared_mutex>
#include <string>

namespace mini {
///////////////////////////////////////////////////////////////////////////
/// Useful Aliases
constexpr unsigned int ASSETMANAGER_MAX_THREADS = 8u;
using Asset_Work_Order = std::function<void(void)>;

///////////////////////////////////////////////////////////////////////////
/// \class AssetManager
/// \brief  Manages the storage and retrieval of assets.
class AssetManager {
    public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Checks if an asset already exists with the given filename,
    /// fetching if true.
    /// \param	assetType		the name of the asset type to search for.
    /// \param	filename		the asset's relative filename .
    /// \param	constructor		construction method for creating the asset.
    /// \param  threaded		flag to create in a separate thread.
    /// \return the asset, if found, or nullptr otherwise.
    [[nodiscard]] Shared_Asset shareAsset(
        const std::string& assetType, const std::string& filename,
        const std::function<Shared_Asset(void)>& constructor,
        const bool& threaded);
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Pop's the first work order and completes it. */
    void beginWorkOrder();
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Forwards notification callbacks for the main-thread.
    /// \param  callback        the callback parameters.
    void submitNotifyee(
        const std::pair<std::shared_ptr<bool>, std::function<void()>>&
            callback);
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Calls all relevant finalization callbacks.
    /// \note   Call only from the main thread!
    void notifyObservers();
    ///////////////////////////////////////////////////////////////////////////
    /// \brief   Retrieves whether or not this manager is ready to use.
    /// \return	true if all work is finished, false otherwise.
    bool readyToUse();

    private:
    ///////////////////////////////////////////////////////////////////////////
    /// Private Attributes
    std::shared_mutex m_mutexAssetMap; ///< Mutex for the asset map.
    std::map<std::string, std::vector<Shared_Asset>>
        m_assetMap;                            ///< Container for assets.
    std::shared_mutex m_mutexWorkorders;       ///< Mutex for asset work orders.
    std::deque<Asset_Work_Order> m_workOrders; ///< Container for work orders.
    std::shared_mutex m_mutexNofications;      ///< Mutex for notifications.
    std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>>
        m_notifyees; ///< Container for notifications.
};
};     // namespace mini
#endif // MINI_ASSETMANAGER_HPP