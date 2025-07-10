#pragma once

#include "Debug/Logging.h"

#include <string>
#include <cstdint>

namespace BC
{
    
    namespace Util
    {
        using ShaderDataType = uint8_t;
        enum : ShaderDataType { Unknown = 0, ShaderDataType_Float, ShaderDataType_Float2, ShaderDataType_Float3, ShaderDataType_Float4, ShaderDataType_Mat3, ShaderDataType_Mat4, ShaderDataType_Int, ShaderDataType_Int2, ShaderDataType_Int3, ShaderDataType_Int4, ShaderDataType_Bool };
        
        static size_t ShaderDataTypeSize(ShaderDataType type)
        {
            switch (type)
            {
                case ShaderDataType_Float:  return sizeof(float);
                case ShaderDataType_Float2: return sizeof(float) * 2;
                case ShaderDataType_Float3: return sizeof(float) * 3;
                case ShaderDataType_Float4: return sizeof(float) * 4;
                case ShaderDataType_Mat3:   return sizeof(float) * 3 * 3;
                case ShaderDataType_Mat4:   return sizeof(float) * 4 * 4;
                case ShaderDataType_Int:    return sizeof(int);
                case ShaderDataType_Int2:   return sizeof(int) * 2;
                case ShaderDataType_Int3:   return sizeof(int) * 3;
                case ShaderDataType_Int4:   return sizeof(int) * 4;
                case ShaderDataType_Bool:   return sizeof(bool);
            }
            BC_CORE_WARN("ShaderDataTypeSize: Shader Data Type Not Defined!");
            return 0;
        }

    }

    struct BufferLayoutElement
    {

        BufferLayoutElement() = default;
        BufferLayoutElement(Util::ShaderDataType type, const std::string& name, bool normalised = false) :
            data_name(name),
            data_type(type),
            data_size(Util::ShaderDataTypeSize(type)),
            data_offset(0),
            data_normalised(normalised) { }

        size_t GetComponentCount() const 
        {
			switch (data_type)
			{
                case Util::ShaderDataType_Float:   return 1;
                case Util::ShaderDataType_Float2:  return 2;
                case Util::ShaderDataType_Float3:  return 3;
                case Util::ShaderDataType_Float4:  return 4;
                case Util::ShaderDataType_Mat3:    return 3;
                case Util::ShaderDataType_Mat4:    return 4;
                case Util::ShaderDataType_Int:     return 1;
                case Util::ShaderDataType_Int2:    return 2;
                case Util::ShaderDataType_Int3:    return 3;
                case Util::ShaderDataType_Int4:    return 4;
                case Util::ShaderDataType_Bool:    return 1;
			}

			BC_CORE_WARN("BufferLayoutElement::GetComponentCount: Shader Data Type Not Defined!");
			return 0;
        }

        std::string data_name;
        Util::ShaderDataType data_type;
        size_t data_size;
        size_t data_offset;
        bool data_normalised;
    };

    class BufferLayout
    {

    public:

		BufferLayout() { }
		BufferLayout(std::initializer_list<BufferLayoutElement> elements) : 
            m_Elements(elements) 
        {
			SetupBufferLayout();
		}

		size_t GetStride() const { return m_Stride; }
		const std::vector<BufferLayoutElement>& GetElements() const { return m_Elements; }

		std::vector<BufferLayoutElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferLayoutElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferLayoutElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferLayoutElement>::const_iterator end() const { return m_Elements.end(); }
    
    private:

        void SetupBufferLayout()
        {
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.data_offset = offset;
				offset += element.data_size;
				m_Stride += element.data_size;
			}
        }
    
    private:

        size_t m_Stride = 0;
        std::vector<BufferLayoutElement> m_Elements = {};
    };

}