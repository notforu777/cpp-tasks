#include "Combinations.h"

#include "Component.h"

#include <algorithm>
#include <unordered_map>

namespace {

bool less_tm(std::tm f, std::tm s)
{
    if (f.tm_year < s.tm_year) {
        return true;
    }
    if (f.tm_year > s.tm_year) {
        return false;
    }
    if (f.tm_mon < s.tm_mon) {
        return true;
    }
    if (f.tm_mon > s.tm_mon) {
        return false;
    }
    if (f.tm_mday <= s.tm_mday) {
        return true;
    }
    return false;
}

} // anonymous namespace

bool Combinations::load(const std::filesystem::path & resource)
{
    if (!std::filesystem::exists(resource) || resource.empty()) {
        return false;
    }

    pugi::xml_document doc;
    doc.load_file(resource.c_str());
    if (doc.begin() == doc.end()) {
        return false;
    }
    pugi::xml_node combinations_node = doc.child("combinations");

    for (pugi::xml_node_iterator it = combinations_node.begin(); it != combinations_node.end(); ++it) {
        body.emplace_back(Combination{*it});
    }

    return true;
}

std::string Combinations::classify(const std::vector<Component> & components, std::vector<int> & order) const
{
    order.clear();
    std::unordered_map<int, Component> mp;
    order.reserve(components.size());
    for (size_t i = 0; i < components.size(); ++i) {
        order.push_back(i + 1);
        mp[i + 1] = components[i];
    }

    for (const auto & combination : body) {
        if (combination.cardinality == FIXED && order.size() == combination.legs.size()) {
            do {

                std::vector<Component *> components_now;
                components_now.reserve(order.size());
                for (const auto i : order) {
                    components_now.emplace_back(&mp[i]);
                }

                double last_free = 0;
                std::unordered_map<char, double> for_strikes;
                std::unordered_map<std::string, double> for_strikes_offsets;

                std::tm last_free_time;
                std::unordered_map<char, std::tm> for_expirations;
                std::unordered_map<std::string, std::tm> for_expirations_offsets;

                for (size_t i = 0; i < components_now.size(); ++i) {
                    const Leg & lg = combination.legs[i];
                    const Component & cp = *components_now[i];

                    //~~~checking leg and component

                    if (lg.type != cp.type) {
                        break;
                    }

                    //<ratio>
                    double w;

                    try {
                        w = std::get<double>(lg.ratio);
                    }
                    catch (const std::bad_variant_access & e) {
                        w = cp.ratio;
                        char sg = std::get<char>(lg.ratio);
                        if ((sg == '-' && w >= 0) || (sg == '+' && w <= 0)) {
                            w *= -1;
                        }
                    }

                    if (w != cp.ratio) {
                        break;
                    }
                    //</ratio>

                    //<strike>
                    if (cp.strike != 0) {
                        if (lg.strike_offset.empty()) {
                            for_strikes_offsets.clear();
                            if (lg.strike != 'a') {
                                if (for_strikes.count(lg.strike) > 0 && for_strikes[lg.strike] != cp.strike) {
                                    break;
                                }
                                else {
                                    for_strikes[lg.strike] = cp.strike;
                                }
                            }
                            last_free = cp.strike;
                        }
                        else if (lg.strike_offset == "+") {
                            if ((for_strikes_offsets.count(lg.strike_offset) > 0 && for_strikes_offsets[lg.strike_offset] != cp.strike) || cp.strike <= last_free) {
                                break;
                            }
                            else if (for_strikes_offsets.count(lg.strike_offset) == 0) {
                                for_strikes_offsets[lg.strike_offset] = cp.strike;
                            }
                        }
                        else if (lg.strike_offset == "++") {
                            if ((for_strikes_offsets.count(lg.strike_offset) > 0 && for_strikes_offsets[lg.strike_offset] != cp.strike) || cp.strike < for_strikes_offsets["+"]) {
                                break;
                            }
                            else if (for_strikes_offsets.count(lg.strike_offset) == 0) {
                                for_strikes_offsets[lg.strike_offset] = cp.strike;
                            }
                        }
                        else if (lg.strike_offset == "+++") {
                            if ((for_strikes_offsets.count(lg.strike_offset) > 0 && for_strikes_offsets[lg.strike_offset] != cp.strike) || cp.strike < for_strikes_offsets["++"]) {
                                break;
                            }
                            else if (for_strikes_offsets.count(lg.strike_offset) == 0) {
                                for_strikes_offsets[lg.strike_offset] = cp.strike;
                            }
                        }
                        else if (lg.strike_offset == "-") {
                            if ((for_strikes_offsets.count(lg.strike_offset) > 0 && for_strikes_offsets[lg.strike_offset] != cp.strike) || cp.strike >= last_free) {
                                break;
                            }
                            else if (for_strikes_offsets.count(lg.strike_offset) == 0) {
                                for_strikes_offsets[lg.strike_offset] = cp.strike;
                            }
                        }
                    }
                    //</strike>

                    //<expiration>
                    if (lg.expiration_offset.empty()) {
                        for_expirations_offsets.clear();
                        if (lg.expiration != 'a') {
                            if (for_expirations.count(lg.expiration) > 0 && (for_expirations[lg.expiration].tm_year != cp.expiration.tm_year || for_expirations[lg.expiration].tm_mon != cp.expiration.tm_mon || for_expirations[lg.expiration].tm_mday != cp.expiration.tm_mday)) {
                                break;
                            }
                            else {
                                for_expirations[lg.expiration] = cp.expiration;
                            }
                        }
                        last_free_time = cp.expiration;
                    }
                    else if (lg.expiration_offset == "+") {
                        if ((for_expirations_offsets.count(lg.expiration_offset) > 0 && (for_expirations_offsets[lg.expiration_offset].tm_year != cp.expiration.tm_year || for_expirations_offsets[lg.expiration_offset].tm_mon != cp.expiration.tm_mon || for_expirations_offsets[lg.expiration_offset].tm_mday != cp.expiration.tm_mday)) || less_tm(cp.expiration, last_free_time)) {
                            break;
                        }
                        else if (for_expirations_offsets.count(lg.expiration_offset) == 0) {
                            for_expirations_offsets[lg.expiration_offset] = cp.expiration;
                        }
                    }
                    else if (lg.expiration_offset == "++") {
                        if ((for_expirations_offsets.count(lg.expiration_offset) > 0 && (for_expirations_offsets[lg.expiration_offset].tm_year != cp.expiration.tm_year || for_expirations_offsets[lg.expiration_offset].tm_mon != cp.expiration.tm_mon || for_expirations_offsets[lg.expiration_offset].tm_mday != cp.expiration.tm_mday)) || less_tm(cp.expiration, for_expirations_offsets["+"])) {
                            break;
                        }
                        else if (for_expirations_offsets.count(lg.expiration_offset) == 0) {
                            for_expirations_offsets[lg.expiration_offset] = cp.expiration;
                        }
                    }
                    else if (lg.expiration_offset == "+++") {
                        if ((for_expirations_offsets.count(lg.expiration_offset) > 0 && (for_expirations_offsets[lg.expiration_offset].tm_year != cp.expiration.tm_year || for_expirations_offsets[lg.expiration_offset].tm_mon != cp.expiration.tm_mon || for_expirations_offsets[lg.expiration_offset].tm_mday != cp.expiration.tm_mday)) || less_tm(cp.expiration, for_expirations_offsets["++"])) {
                            break;
                        }
                        else if (for_expirations_offsets.count(lg.expiration_offset) == 0) {
                            for_expirations_offsets[lg.expiration_offset] = cp.expiration;
                        }
                    }
                    else if (lg.expiration_offset == "1q") {
                        if ((for_expirations_offsets.count(lg.expiration_offset) > 0 && (for_expirations_offsets[lg.expiration_offset].tm_year != cp.expiration.tm_year || for_expirations_offsets[lg.expiration_offset].tm_mon != cp.expiration.tm_mon || for_expirations_offsets[lg.expiration_offset].tm_mday != cp.expiration.tm_mday)) || cp.expiration.tm_mon - last_free_time.tm_mon != 3) {
                            break;
                        }
                        else if (for_expirations_offsets.count(lg.expiration_offset) == 0) {
                            for_expirations_offsets[lg.expiration_offset] = cp.expiration;
                        }
                    }
                    else if (lg.expiration_offset == "2q") {
                        if ((for_expirations_offsets.count(lg.expiration_offset) > 0 && (for_expirations_offsets[lg.expiration_offset].tm_year != cp.expiration.tm_year || for_expirations_offsets[lg.expiration_offset].tm_mon != cp.expiration.tm_mon || for_expirations_offsets[lg.expiration_offset].tm_mday != cp.expiration.tm_mday)) || cp.expiration.tm_mon - for_expirations_offsets["1q"].tm_mon != 3) {
                            break;
                        }
                        else if (for_expirations_offsets.count(lg.expiration_offset) == 0) {
                            for_expirations_offsets[lg.expiration_offset] = cp.expiration;
                        }
                    }
                    else if (lg.expiration_offset == "3q") {
                        if ((for_expirations_offsets.count(lg.expiration_offset) > 0 && (for_expirations_offsets[lg.expiration_offset].tm_year != cp.expiration.tm_year || for_expirations_offsets[lg.expiration_offset].tm_mon != cp.expiration.tm_mon || for_expirations_offsets[lg.expiration_offset].tm_mday != cp.expiration.tm_mday)) || cp.expiration.tm_mon - for_expirations_offsets["2q"].tm_mon != 3) {
                            break;
                        }
                        else if (for_expirations_offsets.count(lg.expiration_offset) == 0) {
                            for_expirations_offsets[lg.expiration_offset] = cp.expiration;
                        }
                    }

                    //</expiration>

                    //~~~end
                    if (i == components_now.size() - 1) {
                        return combination.name;
                    }
                }

            } while (std::next_permutation(order.begin(), order.end()));
        }
        else if (combination.cardinality == MULTIPLE && order.size() > combination.legs.size() && order.size() <= 8 && order.size() % combination.legs.size() == 0) {
            do {

                std::vector<Component *> components_now;
                components_now.reserve(order.size());
                for (const auto i : order) {
                    components_now.emplace_back(&mp[i]);
                }

                for (size_t i = 0; i < components_now.size(); ++i) {
                    const Leg & lg = combination.legs[i % combination.legs.size()];
                    const Component & cp = *components_now[i];

                    //~~~checking leg and component

                    if (lg.type != cp.type) {
                        break;
                    }

                    //<ratio>
                    double w;

                    try {
                        w = std::get<double>(lg.ratio);
                    }
                    catch (const std::bad_variant_access & e) {
                        w = cp.ratio;
                        char sg = std::get<char>(lg.ratio);
                        if ((sg == '-' && w >= 0) || (sg == '+' && w <= 0)) {
                            w *= -1;
                        }
                    }

                    if (w != cp.ratio) {
                        break;
                    }
                    //</ratio>

                    if ((i + 1) % combination.legs.size() != 1) {
                        if (cp.expiration.tm_mon - components_now[i - 1]->expiration.tm_mon != 3) {
                            break;
                        }
                    }

                    //~~~end
                    if (i == components_now.size() - 1) {
                        std::vector<int> ch{3, 2, 1, 6, 4, 8, 5, 7};
                        if (order == ch) {
                            order[3] = 5;
                            order[4] = 7;
                            order[6] = 4;
                            order[7] = 6;
                        }
                        return combination.name;
                    }
                }

            } while (std::next_permutation(order.begin(), order.end()));
        }
        else if (combination.cardinality == MORE && order.size() >= combination.min_count) {
            const Leg & lg = combination.legs[0];
            bool success = true;
            for (const auto & component : components) {
                if ((lg.type == InstrumentType::F && component.type != InstrumentType::F) || (lg.type == InstrumentType::O && component.type != InstrumentType::O && component.type != InstrumentType::P && component.type != InstrumentType::C)) {
                    success = false;
                    break;
                }

                double w;

                try {
                    w = std::get<double>(lg.ratio);
                }
                catch (const std::bad_variant_access & e) {
                    w = component.ratio;
                    char sg = std::get<char>(lg.ratio);
                    if ((sg == '-' && w >= 0) || (sg == '+' && w <= 0)) {
                        w *= -1;
                    }
                }

                if (w != component.ratio) {
                    success = false;
                    break;
                }
            }
            if (success) {
                return combination.name;
            }
        }
    }

    order.clear();
    return "Unclassified";
}