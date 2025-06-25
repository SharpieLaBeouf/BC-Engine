#pragma once
#include "BC.h"

namespace BC
{
    class SelectionContext
    {
        
    public:

        template<typename T>
        void SetSelection(const T& value)
        {
            m_Selected = std::make_shared<std::decay_t<T>>(value);
            m_Type = &typeid(std::decay_t<T>);
        }

        template<typename T>
        void SetSelection(std::shared_ptr<T> value)
        {
            m_Selected = value;
            m_Type = &typeid(T);
        }

        template<typename T>
        const T& GetSelection() const
        {
            if (*m_Type == typeid(T) && m_Selected)
                return *static_cast<T*>(m_Selected.get());

            static const T null_value{};
            return null_value;
        }

        void Clear() { m_Selected.reset(); m_Type = &typeid(void); }

    private:

        std::shared_ptr<void> m_Selected;
        const std::type_info* m_Type = &typeid(void);
    };
}