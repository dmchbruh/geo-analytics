#pragma once

#include <optional>
#include <string>
#include <vector>

struct OsmTag
{
    std::string key;
    std::string value;
};

struct BusinessCategory
{
    std::string name;
    std::vector<OsmTag> competitorTags;
    std::vector<OsmTag> demandTags;
};

std::optional<BusinessCategory> findBusinessCategory(const std::string& name);

std::string toOverpassFilter(const OsmTag& tag);

std::string toOsmiumFilter(const OsmTag& tag);