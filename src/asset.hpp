#pragma once
#ifndef MINI_ASSET_HPP
#define MINI_ASSET_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace mini {
///////////////////////////////////////////////////////////////////////////
/// Useful Aliases
class Asset;
class AssetManager;
using Shared_Asset = std::shared_ptr<Asset>;
using AssetFinalizedCallback = std::function<void(void)>;

///////////////////////////////////////////////////////////////////////////
/// \class  Asset
/// \brief  An abstract base-class for assets to be loaded from disk once.
class Asset {
    public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Destroy this Asset.
    inline virtual ~Asset() = default;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Retrieves the file name of this asset.
    /// \return	the file name belonging to this asset.
    std::string getFileName() const { return m_filename; }
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Attaches a callback for when the asset finishes loading.
    /// \param	alive		a shared bool indicating whether the caller is
    /// still. \param	callback	the method to be triggered.
    void addCallback(
        const std::shared_ptr<bool>& alive,
        const AssetFinalizedCallback& callback);
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Retrieves whether or not this asset has completed finalizing.
    /// \return	true if this asset has finished finalizing, false otherwise.
    bool ready() const noexcept;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Check if an input variadic list of shared assets have all
    /// completed finalizing.
    /// \tparam	<>			variadic list of assets to check (auto-deducible).
    /// \param	firstAsset	the first value to check.
    /// \param	...rest		the rest of the values to check.
    /// \return				true if all the assets have finished finalizing,
    /// false otherwise.
    template <typename FirstAsset, typename... RemainingAssets>
    static bool All_Ready(
        const FirstAsset& firstAsset, const RemainingAssets&... rest) noexcept {
        // Ensure all inputs are shared assets
        static_assert(
            !std::is_base_of<std::shared_ptr<Asset>, FirstAsset>::value,
            "Asset::All_Ready(...) parameter is not a Shared_Asset!");

        // Proceed only if the first asset is ready, recursively
        if (firstAsset->ready()) {
            // For each remaining member of the parameter pack,
            // recursively call this function
            if constexpr (sizeof...(rest) > 0)
                return All_Ready(rest...);
            else
                return true;
        }

        return false;
    }

    protected:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Create asset that uses the specified file-path.
    /// \param  assetManager    the asset manager.
    /// \param  filename        the relative file name.
    Asset(AssetManager& assetManager, const std::string& filename)
        : m_assetManager(assetManager), m_filename(filename) {}

    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Initializes the asset.
    virtual void initialize() = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Declares this asset ready-to-use.
    void finalize();

    ///////////////////////////////////////////////////////////////////////////
    /// Protected Attributes
    AssetManager& m_assetManager;
    std::atomic_bool m_finalized = false; ///< Flag indicating finalized.
    std::string m_filename = "";          ///< Relative file name.
    std::vector<std::pair<std::shared_ptr<bool>, std::function<void()>>>
        m_callbacks;           ///< Vector of callbacks on finalization.
    friend class AssetManager; ///< AssetManager can access these members.

    private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Disallow asset default constructor.
    ///////////////////////////////////////////////////////////////////////////
    Asset() noexcept = delete;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Disallow asset move constructor.
    ///////////////////////////////////////////////////////////////////////////
    Asset(Asset&&) noexcept = delete;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Disallow asset copy constructor.
    ///////////////////////////////////////////////////////////////////////////
    Asset(const Asset&) noexcept = delete;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Disallow asset move assignment.
    ///////////////////////////////////////////////////////////////////////////
    Asset& operator=(Asset&&) noexcept = delete;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief  Disallow asset copy assignment.
    Asset& operator=(const Asset&) noexcept = delete;
};
};     // namespace mini
#endif // MINI_ASSET_HPP