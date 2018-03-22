#pragma once

#include <hdf5.h>

#include "AbstractDataSet.h"
#include "Attribute.h"
#include "DataSet.h"
#include "DataSpace.h"
#include "DataType.h"
#include "File.h"
#include "Group.h"
#include "IdComponent.h"
#include "Location.h"
#include "Object.h"
#include "PropList.h"

/// Wraps HDF5 C API.
///
/// Based on HDF5 C++ API, which is not used since it doesn't support parallel
/// I/O. Some ideas were also taken from the HighFive project
/// (https://github.com/BlueBrain/HighFive).
namespace HDF5 {}
