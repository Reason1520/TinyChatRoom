#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
#include <mutex>
#include <iostream>

/******************************************************************************
 * @file       singleton.h
 * @brief      单例类模板
 *
 * @author     lueying
 * @date       2025/12/8
 * @history
 *****************************************************************************/

template<typename T>
class Singleton {
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;                // 禁止拷贝构造函数
    Singleton& operator=(const Singleton<T>&) = delete;     // 禁止拷贝赋值运算符

    static std::shared_ptr<T> instance_;
public:
    static std::shared_ptr<T> getInstance() {
        static std::once_flag created;
        std::call_once(created, []() {
            // 使用 new 在 Singleton 的上下文中构造 T，确保友元权限可以访问私有构造函数
            instance_.reset(new T());
            });

        return instance_;
    }

    void printAddress() {
        std::cout << instance_.get() << std::endl;
    }
    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;

#endif // SINGLETON_H