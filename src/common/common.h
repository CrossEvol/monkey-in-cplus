//
// Created by mizuk on 2024/12/3.
//

#ifndef COMMON_H
#define COMMON_H

template<typename Base, typename Derived>
const char *get_type_id_name([[maybe_unused]] Base &base) {
    static_assert(std::is_base_of_v<Base, Derived>, "Derived must be a derived class of Base");
    Derived *derived = dynamic_cast<Derived *>(&base);
    if (derived) {
        return typeid(*derived).name();
    }
    return nullptr;
}

template<typename Base, typename Derived>
bool instance_of([[maybe_unused]] Base &base) {
    static_assert(std::is_base_of_v<Base, Derived>, "Derived must be a derived class of Base");
    return get_type_id_name<Base, Derived>(base) == typeid(Derived).name();
}

#endif //COMMON_H
