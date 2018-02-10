#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <memory>
using namespace std;

class Tag;

class Tag
{
public:
    explicit Tag(const string& name_):name(name_){}
    map<string,shared_ptr<Tag>>children;
    string name;
    map<string, string>attribute;
};


void left_trim(string& s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch){
        return !isspace(ch);
    }));
}

void right_trim(string& s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](int ch){
        return !isspace(ch);
    }).base(), s.end());
}

map<string, string> find_attributes(const string& tag_string)
{
    map<string, string> ret;
    size_t eq_pos = tag_string.find('=');
    size_t cur_pos = 0;
    while (eq_pos != string::npos) {
        string attr_name = tag_string.substr(cur_pos, eq_pos-cur_pos);
        left_trim(attr_name);
        right_trim(attr_name);
        size_t first_q_pos = tag_string.find('\"', eq_pos);
        size_t last_q_pos = tag_string.find('\"', first_q_pos+1);

        string attr_val = tag_string.substr(first_q_pos+1, last_q_pos-first_q_pos-1);
        cur_pos = last_q_pos+1;
        eq_pos = tag_string.find('=', cur_pos+1);
        ret[attr_name]=attr_val;
    }

    return ret;
};

size_t find_start_tag(const string& str, size_t offset = 0)
{
    size_t pos = str.find('<', offset);
    size_t end_pos = str.find("</", offset);

    while (pos==end_pos) {
        pos = str.find('<', end_pos+1);
        if (pos == string::npos)
            break;
        end_pos = str.find("</", end_pos+1);
    }
    return  pos;
}

map<string, shared_ptr<Tag>> parse_hrml(const string& ss)
{
    map<string, shared_ptr<Tag>> rootMap;

    size_t start_tag_pos = find_start_tag(ss, 0);
    while (start_tag_pos < string::npos) {
        size_t end_of_tag_name_pos = ss.find(' ', start_tag_pos);
        string cur_tag_name = ss.substr(start_tag_pos + 1, end_of_tag_name_pos - start_tag_pos - 1);

        string end_tag_token = string{"</"} + cur_tag_name + string{">"};
        size_t end_tag_pos = ss.find(end_tag_token);

        shared_ptr<Tag>root = make_shared<Tag>(cur_tag_name);
        rootMap[root->name]=root;

        // find attributes
        size_t closing_brace = ss.find('>', start_tag_pos);
        root->attribute = find_attributes(ss.substr(end_of_tag_name_pos, closing_brace-end_of_tag_name_pos));

        start_tag_pos = find_start_tag(ss, start_tag_pos+1);
        if (start_tag_pos > end_tag_pos) { // sibling
            rootMap[root->name]=root;
        } else if (start_tag_pos < string::npos) {
            root->children = parse_hrml(ss.substr(closing_brace + 1, end_tag_pos - closing_brace - 1));
        }
        start_tag_pos = find_start_tag(ss, end_tag_pos);
    }

    return rootMap;
}

void find_and_print_response(map<string, shared_ptr<Tag>>rootTag, const string & query)
{
    if (rootTag.empty())
        return;
    vector<string> tag_names;
    string attribute;
    size_t found = query.find('~');
    if (found<string::npos)
        attribute = query.substr(found+1);
    string tags_string = query.substr(0,found);

    string cur_tag_name{};
    string delimiter = string{"."};
    do {
        found = tags_string.find(delimiter);
        cur_tag_name = tags_string.substr(0, found);
        tag_names.push_back(cur_tag_name);
        cerr << "cur_tag_name: " << cur_tag_name << "\n";

        tags_string.erase(0, found+1);
    } while (found < string::npos);

    shared_ptr<Tag> cur_tag = rootTag.begin()->second;
    if (tag_names.at(0) != cur_tag->name) {
        cout << "Not Found!\n";
        return;
    }
    for (vector<Tag>::size_type i = 1; i < tag_names.size(); ++i) {
        auto childIt = cur_tag->children.find(tag_names.at(i));
        if (childIt == cur_tag->children.end()) {
            cout << "Not Found!\n";
            return;
        }
        cur_tag = childIt->second;
    }

    auto attrIt = cur_tag->attribute.find(attribute);
    if (attrIt == cur_tag->attribute.end()) {
        cout << "Not Found!\n";
        return;
    }
    cout << attrIt->second << "\n";
}

int main() {
    int numLines {0}, numQ{0};
    cin >> numLines >> numQ;
    cin.ignore();

    string cur_hrml, hrml;

    for (int i = 0; i < numLines; ++i) {
        getline(cin, cur_hrml);
        hrml+=cur_hrml+"\n";
    }

    vector<string> queries;
    string query_for_print;
    for (int i = 0; i < numQ; ++i) {
        getline(cin,cur_hrml);
        queries.push_back(cur_hrml);
        query_for_print += cur_hrml+"\n";
    }

    map<string,shared_ptr<Tag>> rootTag = parse_hrml(hrml);

    for (const auto query : queries)
        find_and_print_response(rootTag, query);

    return 0;
}
