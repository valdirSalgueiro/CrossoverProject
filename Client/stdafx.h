#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN          

#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "spdlog/spdlog.h"
#include "client_http.hpp"
#include "server_http.hpp"
#include "json.hpp"
#include <queue>
#include <Psapi.h>
#include <iostream>
#include <strsafe.h>
#include "tinyxml2.h"
#include "pdh.h"

#include <objidl.h>
#include <gdiplus.h>
