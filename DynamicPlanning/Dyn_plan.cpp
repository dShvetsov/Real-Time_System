#include <libxml++/libxml++.h>
#include <libxml++/document.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>

/**
 * @mainpage Задание по спецкурсу ВИУС РВ
 * Программа выполняющая моделирование динамического планирования работы однопроцессной
 * вычислительной системы реального времени без прерывания задач с частотно монотонной схемой
 * @autor Денис Швецов
 * МГУ им. М.В. Ломоносова
 * ф-т ВМК
 * группа 205
 */

/**
 * @brief description of the task
 */
class task
{
    std::string name;
    int period;
    int duration;
    double priority;
    int remained_time; //time which remained for execution in this period
    bool perfom; //Did this task perfom in this period? True if not, False if yes
public:
    /**
     * @brief constructor.
     * @param Nm name of task.
     * @param per period of task.
     * @param dur duration of task.
     */
    task (const std::string& Nm, int per, int dur):name(Nm), period(per), duration(dur)
    {
        if (per) priority = 1 / double(per);
        remained_time = period;
        perfom = TRUE;
    }
    /**
     * @brief constructor copy.
     */
    task(const task &a):period(a.period), duration(a.duration), priority(a.priority),
                        remained_time(a.remained_time), perfom(a.perfom)
    {
        name = a.name;
    }
    /**
     * @brief get name of task.
     * @return name of task.
     */
    const std::string &get_name() const {return name;}
    /**
     * @brief get possibility of execution.
     * @return Can task execute in this period or not.
     */
    bool can_perfom() const {return (remained_time >= duration && perfom);}
    /** @brief Execute this task. */
    void doit(){perfom = FALSE;}
    /**
     * @brief substructin of time
     * @detailed this function models time for task
     * @param t = 1 time
     */
    void dec_time(int t = 1){
        if (remained_time - t <= 0)
        {
            // If new period starts task is not perfomed
            perfom = TRUE;
            remained_time = period + (remained_time - t) % period;
        }
        else
            remained_time = (remained_time - t) % period;
    }
    /**
     * @brief get priority of task
     */
    int get_priority() const {return priority;}
    friend bool pr_cmp(const task& a,const  task& b);
    /**
     * @brief compare two task
     * @return TRUE if task is equal
     */
    bool operator == (task a)
    {
        return (name == a.name && period == a.period);
    }
    /**
     * @brief get duration of task
     */
    int get_duration() const{return duration;}
};

/**
 * @brief compare priority of task
 */
bool pr_cmp(const task& a,const task& b){return a.priority > b.priority ;}

task nonaName ("0", 0, 0);
task &NullTask = nonaName; //task, which show, that don't exist task, that must be execute

/**
 * @brief List of tasks, that are executing on processor
 */
class ListOfTask
{
    std::vector<task> tasks;
public:
    /**
     * @brief add task in the list
     * @param a task
     */
    void add(const task &a){ tasks.push_back(a); }
    /**
     * @brief sort list by priority of task
     */
    void priority_sort() { std::sort(tasks.begin(), tasks.end(), pr_cmp); }

    /**
     * @brief substruct time for this tasks
     * @param t time
     */
    void dec_time(int t = 1)
    {
        for (auto p = tasks.begin(); p != tasks.end(); p++)
        {
            (*p).dec_time(t);
        }
    }
    /**
     * @brief get task, that will be execute in this time
     */
    task& get_task()
    {
        for (auto p = tasks.begin(); p != tasks.end(); p++)
        {
            if ((*p).can_perfom())
            {
                (*p).doit();
                return (*p);
            }
        }
        return ::NullTask;
    }
};

/**
 * @brief get task from xml document's node
 * @param node xml document's node
 * @return task
 * @throw reading error if node isn't suit for task
 */
task get_task(const xmlpp::Node* node)
{
    auto nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
    if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
        return ::NullTask;
    std::string Name;
    int period, duration;
    if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node))
    {
        auto attribute = nodeElement->get_attribute("period");
        if(attribute)
        {
            period = std::stoi(std::string(attribute->get_value()));
        }
        else
            throw "have not attribute period";
        attribute = nodeElement->get_attribute("duration");
        if (attribute)
        {
            duration = std::stoi(std::string(attribute->get_value()));
        }
        else
            throw "have not attribute duratiomn";
        attribute = nodeElement->get_attribute("name");
        if (attribute)
        {
            Name = std::string(attribute->get_value());
        }
    }
    else
        throw "problem with cast to Element *";
    return task(Name, period, duration);
}

/**
 * @brief get list of task from xml document
 * @param node xml document's root node
 * @param ans_list list, that collect tasks
 * @return reference to ans_list
 */

ListOfTask& get_list_of_task(const xmlpp::Node* node, ListOfTask & ans_list)
{
    auto list = node->get_children();
    for(auto iter = list.begin(); iter != list.end(); ++iter)
    {
        task ins = get_task(*iter);
        if (!(ins == NullTask))
            ans_list.add(ins);
    }
    return ans_list;
}

int main(int argc, char ** argv)
{
    ListOfTask MainList;
    Glib::ustring filepath;
    if(argc > 1 )
        filepath = argv[1]; //Allow the user to specify a different XML file to parse.
    else
        filepath = "input_dynamic_rm.xml";
    unsigned int runtime;
    try{
        xmlpp::DomParser parser;
        parser.set_substitute_entities();
        parser.parse_file(filepath);
        if (parser)
        {
            xmlpp::Node* pNode = parser.get_document()->get_root_node();
            xmlpp::Element* nodeElement = dynamic_cast<xmlpp::Element*>(pNode);
            xmlpp::Attribute* runtime_s = nodeElement->get_attribute("runtime");
            runtime = std::stoi(std::string(runtime_s->get_value()));
            MainList = get_list_of_task(pNode, MainList);
        }}
    catch(const std::exception& ex)
    {
        std::cout << "Exception caught: " << ex.what() << std::endl;
    }
    catch(const char * str)
    {
        std::cout << str << std::endl;
    }
    MainList.priority_sort();
    xmlpp::Document mydoc; //creating document, which consist of answer
    xmlpp::Element *pNode_answer = mydoc.create_root_node("trace");


    for (decltype(runtime) t = 0; t <= runtime; t++)
    {
        task cTask = MainList.get_task();
        if (cTask == ::NullTask)
        {
            MainList.dec_time();
        }
        else
        {
            decltype(t) tmp = t + cTask.get_duration() - 1;
            if (tmp > runtime)
                break;
            MainList.dec_time(cTask.get_duration());
            xmlpp::Element *child = pNode_answer->add_child("start");
            child->set_attribute("name", cTask.get_name());
            std::ostringstream oss;
            oss << t;
            child->set_attribute("time", oss.str());
            t =tmp;
        }
    }
    mydoc.write_to_file_formatted("output_dynamic_rm.xml");
    return 0;
}

