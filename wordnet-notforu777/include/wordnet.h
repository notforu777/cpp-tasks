#include <fstream>
#include <limits>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Digraph
{
    std::vector<std::vector<int>> grath;

public:
    Digraph(const std::vector<std::vector<int>> & table_of_nodes, int size)
    {
        if (size > 0) {
            grath.resize(size);
            for (size_t i = 0; i < table_of_nodes.size(); ++i) {
                for (size_t j = 1; j < table_of_nodes[i].size(); ++j) {
                    int main_node = table_of_nodes[i][0];
                    grath[table_of_nodes[i][j]].push_back(main_node);
                    grath[main_node].push_back(table_of_nodes[i][j]);
                }
            }
        }
    }

    const std::vector<std::vector<int>> & get_grath() const
    {
        return grath;
    }
};

class ShortestCommonAncestor
{
    Digraph dg;

    std::vector<int> breadth_first_search(int s) const
    {
        std::vector<int> result;
        if (s > -1) {
            const auto & grath = dg.get_grath();
            result.assign(grath.size(), std::numeric_limits<int>::max());
            result[s] = 0;
            std::queue<int> queue;
            queue.push(s);
            while (!queue.empty()) {
                int current = queue.front();
                queue.pop();
                for (const auto node : grath[current]) {
                    if (result[node] == std::numeric_limits<int>::max()) {
                        result[node] = result[current] + 1;
                        queue.push(node);
                    }
                }
            }
        }
        return result;
    }

public:
    ShortestCommonAncestor(const std::vector<std::vector<int>> & table_of_nodes, int size)
        : dg(Digraph(table_of_nodes, size))
    {
    }

    // calculates length of shortest common ancestor path from node with id 'v' to node with id 'w'
    int length(int v, int w) const
    {
        return breadth_first_search(v)[w];
    }

    // returns node id of shortest common ancestor of nodes v and w
    int ancestor(int v, int w) const
    {
        std::vector<int> distances1 = breadth_first_search(v);
        std::vector<int> distances2 = breadth_first_search(w);
        int min = std::numeric_limits<int>::max();
        int result = -1;
        for (size_t i = 0; i < dg.get_grath().size(); ++i) {
            if (distances1[i] + distances2[i] < min) {
                result = i;
                min = distances1[i] + distances2[i];
            }
        }
        return result;
    }

    // calculates length of shortest common ancestor path from node subset 'subset_a' to node subset 'subset_b'
    int length_subset(const std::set<int> & subset_a, const std::set<int> & subset_b) const
    {
        int min = std::numeric_limits<int>::max();
        int result = -1;
        for (const auto i : subset_a) {
            for (const auto j : subset_b) {
                int current = length(i, j);
                if (current < min) {
                    result = current;
                    min = current;
                }
            }
        }
        return result;
    }

    // returns node id of shortest common ancestor of node subset 'subset_a' and node subset 'subset_b'
    int ancestor_subset(const std::set<int> & subset_a, const std::set<int> & subset_b) const
    {
        int min = std::numeric_limits<int>::max();
        int result = -1;
        for (const auto i : subset_a) {
            for (const auto j : subset_b) {
                int current = ancestor(i, j);
                if (current < min && current != -1) {
                    result = current;
                    min = current;
                }
            }
        }
        return result;
    }
};

class WordNet
{
    int number_of_nodes;
    std::unordered_map<std::string, int> gloss_to_id;
    std::unordered_map<int, std::string> id_to_gloss;
    std::unordered_map<std::string, std::vector<std::string>> noun_to_glosses;
    std::vector<std::vector<int>> table_of_nodes;
    ShortestCommonAncestor helper;

    std::set<int> find_ids_by_glosses(const std::vector<std::string> & glosses)
    {
        std::set<int> ans;
        for (auto i = glosses.begin(); i != glosses.end(); ++i) {
            ans.insert(gloss_to_id[*i]);
        }
        return ans;
    }

public:
    WordNet(const std::string & synsets_fn, const std::string & hypernyms_fn)
        : helper(table_of_nodes, 0)
    {
        std::ifstream l(synsets_fn);
        std::ifstream r(hypernyms_fn);
        if (l.is_open() && r.is_open()) {
            int max_node_id = -1;
            int last_index = 0;
            bool is_first_word = true;
            bool is_synonyms = false;
            int last_node_id = -1;
            std::vector<std::string> synonyms;
            std::string s;

            while (std::getline(l, s)) {
                for (size_t i = 0; i < s.size(); ++i) {
                    if (s[i] == ',') {
                        std::string t = s.substr(last_index, i - last_index);
                        if (is_first_word) {
                            is_first_word = false;
                            is_synonyms = true;
                            last_node_id = std::stoi(t);
                            max_node_id = std::max(max_node_id, last_node_id);
                        }
                        else if (is_synonyms) {
                            synonyms.push_back(t);
                            is_synonyms = false;
                        }
                        last_index = i + 1;
                    }
                    else if (is_synonyms && std::isspace(s[i])) {
                        std::string t = s.substr(last_index, i - last_index);
                        synonyms.push_back(t);
                        last_index = i + 1;
                    }
                    else if (i == s.size() - 1) {
                        is_first_word = true;
                        std::string gloss = s.substr(last_index, i - last_index);
                        for (const auto & word : synonyms) {
                            noun_to_glosses[word].push_back(gloss);
                        }
                        synonyms.clear();
                        gloss_to_id[gloss] = last_node_id;
                        id_to_gloss[last_node_id] = gloss;
                        last_index = 0;
                    }
                }
            }

            number_of_nodes = max_node_id + 1;
            auto & local_table = table_of_nodes;
            last_index = 0;
            std::vector<int> now;

            while (getline(r, s)) {
                for (size_t i = 0; i < s.size(); ++i) {
                    if (s[i] == ',') {
                        std::string t = s.substr(last_index, i - last_index);
                        now.push_back(std::stoi(t));
                        last_index = i + 1;
                    }

                    if (i == s.size() - 1) {
                        std::string t = s.substr(last_index, i + 1 - last_index);
                        now.push_back(std::stoi(t));
                        local_table.push_back(now);
                        now.clear();
                        last_index = 0;
                    }
                }
            }

            helper = {local_table, number_of_nodes};
        }
    }

    class iterator
    {
        using It = std::unordered_map<std::string, std::vector<std::string>>::iterator;
        It m_it;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::string;
        using pointer = const value_type *;
        using reference = const value_type &;
        using iterator_category = std::forward_iterator_tag;

        iterator() = default;

        iterator(It it)
            : m_it(it)
        {
        }

        reference operator*() const
        {
            return m_it->first;
        }

        pointer operator->() const
        {
            return &(m_it->first);
        }

        iterator & operator++()
        {
            m_it++;
            return *this;
        }

        iterator operator++(int)
        {
            auto tmp = *this;
            operator++();
            return tmp;
        }

        friend bool operator==(const iterator & lhs, const iterator & rhs)
        {
            return lhs.m_it == rhs.m_it;
        }

        friend bool operator!=(const iterator & lhs, const iterator & rhs)
        {
            return !(lhs == rhs);
        }
    };

    // get iterator to list all nouns stored in WordNet
    iterator begin()
    {
        return iterator(noun_to_glosses.begin());
    }

    iterator end()
    {
        return iterator(noun_to_glosses.end());
    }

    // returns 'true' if 'word' is stored in WordNet
    bool is_noun(const std::string & word) const
    {
        return noun_to_glosses.count(word) > 0;
    }

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string & noun1, const std::string & noun2)
    {
        return id_to_gloss
                [helper.ancestor_subset(find_ids_by_glosses(noun_to_glosses[noun1]), find_ids_by_glosses(noun_to_glosses[noun2]))];
    }

    // calculates distance between noun1 and noun2
    int distance(const std::string & noun1, const std::string & noun2)
    {
        return helper.length_subset(find_ids_by_glosses(noun_to_glosses[noun1]), find_ids_by_glosses(noun_to_glosses[noun2]));
    }
};

class Outcast
{
    WordNet & wd;

public:
    Outcast(WordNet & wordnet)
        : wd(wordnet)
    {
    }

    // returns outcast word
    std::string outcast(const std::vector<std::string> & nouns) const
    {
        int max = 0;
        std::string ans;
        int counter = 0;
        for (auto i = nouns.begin(); i != nouns.end(); ++i) {
            int d_i = 0;
            for (auto j = nouns.begin(); j != nouns.end(); ++j) {
                if (i != j) {
                    d_i += wd.distance(*i, *j);
                }
            }
            if (d_i > max) {
                ans = *i;
                max = d_i;
                counter = 1;
            }
            else if (d_i == max) {
                ++counter;
            }
        }
        if (counter > 1) {
            ans = "";
        }
        return ans;
    }
};
