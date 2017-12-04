#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include"tinyxml2.h"
using namespace tinyxml2;

struct Task {
    std::string name;
    int period;
    int deadline;
    int duration;
};

using tasks_t = std::vector<Task>;

tasks_t parseXmlFile(const char* filename)
{
    XMLDocument doc;
    if (doc.LoadFile(filename)) {
        throw std::invalid_argument("Error parsing XML doc.");
    }

    auto root = doc.RootElement();
    if (root == nullptr) {
        throw std::invalid_argument("No root element.");
    }
    tasks_t tasks;
    for (auto task_xml = root->FirstChildElement();
         task_xml != nullptr;
         task_xml = task_xml->NextSiblingElement()) {

        Task task;
        auto attr = task_xml->Attribute("name");
        if (attr == nullptr) {
            throw std::invalid_argument("unacceptable name of task");
        }
        task.name = std::string(attr);
        if (task_xml->QueryIntAttribute("period", &task.period) != XML_NO_ERROR) {
            throw std::invalid_argument(
                    std::string("Error reading period for task \"") +
                    task.name + std::string("\".")
                );
        }
        if (task_xml->QueryIntAttribute("deadline", &task.deadline) != XML_NO_ERROR) {
            throw std::invalid_argument(
                    std::string ("Error reading deadline for task \"") +
                    task.name + std::string("\".")
                );
        }
        if (task_xml->QueryIntAttribute("duration", &task.duration) != XML_NO_ERROR) {
            throw std::invalid_argument(
                    std::string ("Error reading duration for task \"") +
                    task.name + std::string("\".")
                );
        }
        tasks.push_back(task);
    }
    return tasks;
}

class Tasks {
public:

    Tasks(tasks_t tasks)
        : m_tasks(std::move(tasks))
    { }
    Tasks(const char* filename)
        : m_tasks(std::move(parseXmlFile(filename)))
    { }

    double util() const
    {
        if (m_util >= 0) {
            return m_util;
        }
        m_util = 0;
        for (const auto& t : m_tasks) {
            m_util += double(t.duration) / t.period;
        }
        return m_util;
    }

    double lmax() const
    {
        if ( std::abs(util() - 1) < 0.00001 ) {
            throw std::logic_error("Upper bound not defined for U=1");
        }

        double acc = 0;
        for (const auto& t : m_tasks) {
            acc += ((t.period - t.deadline) * t.duration / double(t.period));
        }
        auto max_deadline = std::max_element(m_tasks.begin(), m_tasks.end(),
                [](const auto& one, const auto& two){return one.deadline < two.deadline; });
        return std::max(double(max_deadline->deadline), acc / (1 - util()));
    }

    int g_from_zero_to(int L) const {
        int g = 0;
        for (const auto& t : m_tasks) {
            g += std::floor((L - t.deadline + t.period) / double(t.period)) * t.duration;
        }
        return g;
    }

    friend std::ostream& operator<<(std::ostream& o, const Tasks& tasks)
    {
        for (const auto& t : tasks.m_tasks) {
            o << t.name << " : period " << t.period << " deadline " << t.deadline <<
                " duration " << t.duration << std::endl;
        }
        return o;
    }

    unsigned long long period_lcm() const
    {
        using std::lcm;
        unsigned long long cur_lcm = 1;
        for (const auto& t : m_tasks) {
            cur_lcm = lcm(cur_lcm, t.period);
        }
        return cur_lcm;
    }
private:
    tasks_t m_tasks;
    mutable double m_util = -1;

};

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            throw std::invalid_argument("Need to pass filename of input file");
        }
        Tasks tasks(argv[1]);
        double lmax = tasks.lmax();
        bool feasible = true;
        int left = 0, right = 0;
        for (int L = 1; L <= lmax; L++) {
            if (tasks.g_from_zero_to(L) > L) {
                feasible = false;
                left = L;
                break;
            }
        }
        std::cout << (feasible ? "YES" : "NO") << std::endl;
        if (not feasible) {
            right = left;
            unsigned long long lcm_period = tasks.period_lcm();
            bool is_inf = false;
            while (tasks.g_from_zero_to(right) > right) {
                if (right > lcm_period * 2) {
                    is_inf = true;
                    break;
                }
                right++;
            }
            std::cout << left << " " << (is_inf ? "Inf" : std::to_string(right)) << std::endl;
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
