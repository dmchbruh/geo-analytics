#include "config/BusinessCategories.h"

std::optional<BusinessCategory> findBusinessCategory(const std::string& name)
{
    if (name == "coffee_shop")
    {
        return BusinessCategory{
            "coffee_shop",
            { {"amenity", "cafe"} },
            {
                {"office", ""},
                {"building", "residential"},
                {"building", "apartments"},
                {"highway", "bus_stop"}

            },
            {
                { "tourism", "attraction"},
                {"tourism", "museum"},
                {"amenity", "theatre"},
                {"amenity", "cinema"},
                {"leisure", "park"},
                {"shop", "mall"}
            }   
        };
    }

    if (name == "barbershop")
    {
        return BusinessCategory{
            "barbershop",
            { {"shop", "hairdresser"}, {"shop", "barber"} },
            {
                {"building", "residential"},
                {"building", "apartments"},
                {"highway", "bus_stop"}
            },
            {
                { "tourism", "attraction" },
                {"tourism", "museum"},
                {"amenity", "theatre"},
                {"amenity", "cinema"},
                {"leisure", "park"},
                {"shop", "mall"}
            }
        };
    }

    if (name == "restaurant")
    {
        return BusinessCategory{
            "restaurant",
            { {"amenity", "restaurant"}, {"amenity", "fast_food"} },
            {
                {"office", ""},
                {"building", "residential"},
                {"building", "apartments"},
                {"highway", "bus_stop"},
                {"amenity", "cinema"},
                {"amenity", "theatre"}
            },
            {
                { "tourism", "attraction"},
                {"tourism", "museum"},
                {"amenity", "theatre"},
                {"amenity", "cinema"},
                {"leisure", "park"},
                {"shop", "mall"},
                {"amenity", "nightclub"}
            }
        };
    }

    return std::nullopt;
}

std::string toOverpassFilter(const OsmTag& tag)
{
    if (tag.value.empty())
    {
        return "\"" + tag.key + "\"";
    }

    return "\"" + tag.key + "\"=\"" + tag.value + "\"";
}

std::string toOsmiumFilter(const OsmTag& tag)
{
    if (tag.value.empty())
    {
        return tag.key;
    }

    return tag.key + "=" + tag.value;
}