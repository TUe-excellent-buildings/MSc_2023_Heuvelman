#ifndef XML_VECTOR_TRANSLATOR_HPP
#define XML_VECTOR_TRANSLATOR_HPP

#include <boost/optional.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <vector>
#include <list>

namespace BSO { namespace XML {

template<typename T> struct container
    {
        // types
        typedef T internal_type;
        typedef T external_type;

        boost::optional<T> get_value(const std::string& str) const
        {
            if (str.empty())
                return boost::none;

            T values;
            std::stringstream ss(str);

            typename T::value_type temp_value;
            while (ss >> temp_value)
                values.insert(values.end(), temp_value);

            return boost::make_optional(values);
        }

        boost::optional<std::string> put_value(const T& b) {
            std::stringstream ss;
            size_t i = 0;
            for (auto v : b)
                ss << (i++?" ":"") << v;
            return ss.str();
        }
    };

} // XML
} // BSO

namespace boost { namespace property_tree {

template<typename ch, typename traits, typename alloc, typename T>
    struct translator_between<std::basic_string<ch, traits, alloc>, std::vector<T> > {
        typedef BSO::XML::container<std::vector<T> > type;
    };

template<typename ch, typename traits, typename alloc, typename T>
    struct translator_between<std::basic_string<ch, traits, alloc>, std::list<T> > {
        typedef BSO::XML::container<std::list<T> > type;
    };
} // property_tree
} // boost

#endif // XML_VECTOR_TRANSLATOR_HPP
