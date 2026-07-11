// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef USG_GEOM_ASSETPATH_H
#define USG_GEOM_ASSETPATH_H

#include "usg/api.h"
#include "usg/base/Array.h"

#include <iosfwd>
#include <string>
#include <set>
#include <vector>

namespace fdk {
  class Hash;
}

namespace usg {
  //! A wrapper for PXR_NS::SdfAssetPath
  class USG_API AssetPath
  {
  public:
    class Handle; //!< Abstraction handle for casting to underlying implementation type

    AssetPath();
    AssetPath(const std::string& path);
    AssetPath(const std::string& path, const std::string& resolvedPath);
    AssetPath(const AssetPath::Handle& handle);
    ~AssetPath();

    // movable
    AssetPath(AssetPath&& rhs) noexcept;
    AssetPath& operator=(AssetPath&& rhs) noexcept;

    // and copyable
    AssetPath(const AssetPath& rhs);
    AssetPath& operator=(const AssetPath& rhs);

    /// Equality, including the resolved path.
    bool operator==(const AssetPath &rhs) const;
    bool operator!=(const AssetPath &rhs) const;

    /// Ordering first by asset path, then by resolved path.
    bool operator<(const AssetPath &rhs) const;

    //! Returns the asset path.
    const std::string& assetPath() const;

    //! Return the resolved asset path, if any.
    const std::string& resolvedPath() const;

    //! Return the AssetPath as a string.
    const std::string& asString() const;

    //! Get the underlying SdfAssetPath
    AssetPath::Handle&       getAssetPath(int usd_version);
    const AssetPath::Handle& getAssetPath(int usd_version) const;

    //! Get the hash of the underlying SdfAssetPath
    uint64_t hash() const;

    //! Add the path's hash to a fdk::Hash object.
    void append(fdk::Hash& hash) const;

  private:
    int _handle;
  };

  //! Append the path to a hash
  USG_API fdk::Hash& operator<<(fdk::Hash& hash, const AssetPath& path);

  //! Output the path to a stream
  USG_API std::ostream& operator<<(std::ostream& stream, const AssetPath& path);

  using AssetPathArray = usg::Array<AssetPath>;
  using AssetPathSet   = std::set<AssetPath>;
}

#endif
