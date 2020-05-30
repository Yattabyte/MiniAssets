######################
### MiniAssets Lib ###
######################
set(Module MiniAssetsCore)

# Configure and acquire files
set(FILES
    # Header files
    asset.hpp
    assetManager.hpp

    # Source files
    asset.cpp
    assetManager.cpp
)

# Create Library using the supplied files
add_library(${Module} STATIC ${FILES})
target_include_directories(${Module}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# Add library dependencies
target_compile_features(${Module} PRIVATE cxx_std_17)

# Set all project settings
target_compile_Definitions(${Module} PRIVATE $<$<CONFIG:DEBUG>:DEBUG>)
set_target_properties(${Module} PROPERTIES VERSION ${PROJECT_VERSION})