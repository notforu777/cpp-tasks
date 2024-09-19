#include <iostream>
#include <unordered_map>
#include <set>
#include <vector>
#include "wordnet.h"

int main()
{
    WordNet * m_wordnet = new WordNet("tests/etc/synsets.txt", "tests/etc/hypernyms.txt");
    Outcast * m_outcast = new Outcast(*m_wordnet);

    std::cout << m_wordnet->is_noun("whole-word_method") << '\n';

    auto it = m_wordnet->begin();
    auto end = m_wordnet->end();
    size_t n = 0;
    while (it != end) {
        ++n;
        ++it;
    }

    std::vector<std::string> d7 {"Asia", "Australia", "North_America", "India", "Europe", "Antarctica", "South_America"};

    std::cout << m_wordnet->is_noun("genus_Commiphora") << '\n' << n << '\n' << m_wordnet->distance("Alopius", "Alosa")
              << '\n' << m_wordnet->distance("Aarhus", "position") << '\n' << m_outcast->outcast(d7) << '\n';
    std::vector<std::string> d18 {"Nag_Hammadi", "Erythrina_corallodendrum", "digital_camera"};
    std::cout << "1 " << m_outcast->outcast(d18) << '\n';
}
