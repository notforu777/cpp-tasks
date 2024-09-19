#pragma once

#include <filesystem>
#include <iostream>
#include <pugixml.hpp>
#include <string>
#include <variant>
#include <vector>

struct Component;
enum class InstrumentType : char;

class Combinations
{
    struct Leg
    {
        InstrumentType type;
        std::variant<double, char> ratio;
        char strike{'a'};
        std::string strike_offset;
        char expiration{'a'};
        std::string expiration_offset;

        Leg(const pugi::xml_node & leg_node)
        {
            for (pugi::xml_attribute_iterator ait = leg_node.attributes_begin(); ait != leg_node.attributes_end(); ++ait) {
                std::string att_name = ait->name();
                std::string value = ait->value();
                if (att_name == "type") {
                    type = static_cast<InstrumentType>(value[0]);
                }
                else if (att_name == "ratio") {
                    try {
                        ratio = std::stod(value);
                    }
                    catch (std::invalid_argument & e) {
                        ratio = value[0];
                    }
                }
                else if (att_name == "strike") {
                    strike = value[0];
                }
                else if (att_name == "strike_offset") {
                    strike_offset = value;
                }
                else if (att_name == "expiration") {
                    expiration = value[0];
                }
                else if (att_name == "expiration_offset") {
                    expiration_offset = value;
                }
            }
        }
    };

    enum Cardinality
    {
        FIXED,
        MULTIPLE,
        MORE,
        NONE
    };

    struct Combination
    {
        std::string name;
        std::string short_name;
        std::string id;
        Cardinality cardinality{Cardinality::NONE};
        size_t min_count;
        std::vector<Leg> legs;

        Combination(const pugi::xml_node & combination_node)
        {
            for (pugi::xml_attribute_iterator ait = combination_node.attributes_begin(); ait != combination_node.attributes_end(); ++ait) {
                std::string att_name = ait->name();
                std::string value = ait->value();
                if (att_name == "name") {
                    name = value;
                }
                else if (att_name == "short_name") {
                    short_name = value;
                }
                else if (att_name == "id") {
                    id = value;
                }
            }

            pugi::xml_node legs_node = combination_node.child("legs");

            for (pugi::xml_attribute_iterator ait = legs_node.attributes_begin(); ait != legs_node.attributes_end(); ++ait) {
                std::string att_name = ait->name();
                std::string value = ait->value();
                if (att_name == "cardinality") {
                    Cardinality new_one;
                    if (value == "fixed") {
                        new_one = FIXED;
                        cardinality = new_one;
                    }
                    else if (value == "multiple") {
                        new_one = MULTIPLE;
                        cardinality = new_one;
                    }
                    else if (value == "more") {
                        new_one = MORE;
                        cardinality = new_one;
                    }
                }
                else if (att_name == "mincount") {
                    min_count = std::stoi(value);
                }
            }

            for (pugi::xml_node_iterator it = legs_node.begin(); it != legs_node.end(); ++it) {
                legs.emplace_back(Leg{*it});
            }
        }
    };

    std::vector<Combination> body;

public:
    Combinations() = default;

    bool load(const std::filesystem::path & resource);

    std::string classify(const std::vector<Component> & components, std::vector<int> & order) const;
};
