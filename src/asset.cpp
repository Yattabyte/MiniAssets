#include "asset.hpp"
#include "assetManager.hpp"

///////////////////////////////////////////////////////////////////////////
/// Use our shared namespace mini
using namespace mini;

///////////////////////////////////////////////////////////////////////////
/// addCallback
///////////////////////////////////////////////////////////////////////////
void Asset::addCallback(const std::shared_ptr<bool>& alive, const AssetFinalizedCallback& callback) {
    if (!ready()) {
        m_callbacks.emplace_back(std::pair(alive, callback));
    } else {
        callback();
    }
}

///////////////////////////////////////////////////////////////////////////
/// ready
///////////////////////////////////////////////////////////////////////////

bool Asset::ready() const noexcept {
    // Check if we're finalized
    return m_finalized.load();
}

///////////////////////////////////////////////////////////////////////////
/// finalize
///////////////////////////////////////////////////////////////////////////

void Asset::finalize() {
    m_finalized = true;

    // Copy callbacks in case any get added while we're busy
    const auto copyCallbacks = m_callbacks;
    m_callbacks.clear();

    for (const auto& qwe : copyCallbacks) {
        m_assetManager.submitNotifyee(qwe);
    }
}