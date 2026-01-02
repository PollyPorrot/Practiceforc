#include "StudentRepo.h"
#include<algorithm>

bool StudentRepo::addStudent(int id,std::string name,int score){
    if(students_.find(id)!=students_.end())return false;
    students_.emplace(id,Student{id,std::move(name),score});
    return true;
}

const Student* StudentRepo::findById(int id) const{
    auto it = students_.find(id);
    if(it!=students_.end())return &it->second;
    return nullptr;
}

bool StudentRepo::removeById(int id){
    return students_.erase(id) > 0;
}

std::vector<Student> StudentRepo::topK(std::size_t k) const{
    std::vector<Student> v;
    v.reserve(students_.size());
    for(const auto & [id,stu]:students_)v.push_back(stu);
    std::sort(v.begin(),v.end(),[](const Student& a,const Student& b){
        return a.score>b.score;
    });

    if(k<v.size())v.resize(k);
    return v;
}

std::size_t StudentRepo::size() const{
    return students_.size();
}