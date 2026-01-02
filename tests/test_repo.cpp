#include <iostream>
#include "StudentRepo.h"

int main() {
    StudentRepo repo;
    repo.addStudent(1, "Alice", 90);
    repo.addStudent(2, "Bob", 85);
    repo.addStudent(3, "Cindy", 98);

    auto p = repo.findById(2);
    if (!p || p->name != "Bob") return 1;

    auto top2 = repo.topK(2);
    if (top2.size() != 2 || top2[0].score < top2[1].score) return 2;

    if (!repo.removeById(1)) return 3;
    if (repo.size() != 2) return 4;

    std::cout << "Test passed\n";
    return 0;
}
