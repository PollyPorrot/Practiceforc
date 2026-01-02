#pragma once

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <string>

struct Student{
    int id{};
    std::string name;
    int score{};
};

class StudentRepo{
public:
    bool addStudent(int id,std::string name,int score);
    const Student* findById(int id) const;
    bool removeById(int id);
    std::vector<Student> topK(std::size_t k) const;
    std::size_t size() const;
private:
    std::unordered_map<int,Student> students_;
};