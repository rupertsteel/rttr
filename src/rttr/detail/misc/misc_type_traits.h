/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 Axel Menzel <info@axelmenzel.de>                       *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef RTTR_MISC_TYPE_TRAITS_H_
#define RTTR_MISC_TYPE_TRAITS_H_

#include "rttr/detail/base/core_prerequisites.h"

#include "rttr/detail/misc/function_traits.h"
#include "rttr/detail/array/array_mapper.h"
#include "rttr/detail/misc/std_type_traits.h"

#include <type_traits>
#include <memory>

namespace rttr
{

class type;

namespace detail
{
    struct derived_info;

    /////////////////////////////////////////////////////////////////////////////////////////
    // This trait will removes cv-qualifiers, pointers and reference from type T.
    template<typename T, typename Enable = void>
    struct raw_type
    {
        typedef typename detail::remove_cv<T>::type type; 
    };

    template<typename T> struct raw_type<T, typename std::enable_if<std::is_pointer<T>::value && !detail::is_function_ptr<T>::value>::type> 
    {
        typedef typename raw_type<typename detail::remove_pointer<T>::type>::type type; 
    };

    template<typename T> struct raw_type<T, typename std::enable_if<std::is_reference<T>::value>::type> 
    {
        typedef typename raw_type<typename std::remove_reference<T>::type>::type type; 
    };

    template<typename T>
    using raw_type_t = typename raw_type<T>::type;

    /////////////////////////////////////////////////////////////////////////////////////////
    // this trait removes all pointers

    template<typename T, typename Enable = void>
    struct remove_pointers { typedef T type; };
    template<typename T>
    struct remove_pointers<T, typename std::enable_if<std::is_pointer<T>::value>::type>
    {
        typedef typename remove_pointers<typename remove_pointer<T>::type>::type type; 
    };
    
    /////////////////////////////////////////////////////////////////////////////////////////
    // this trait removes all pointers except one

    template<typename T, typename Enable = void>
    struct remove_pointers_except_one { typedef T type; };
    template<typename T>
    struct remove_pointers_except_one<T, typename std::enable_if<std::is_pointer<T>::value>::type>
    {
        typedef typename std::conditional<std::is_pointer<typename remove_pointer<T>::type>::value,
                                          typename remove_pointers_except_one<typename remove_pointer<T>::type>::type,
                                          T>::type type;
    };


    /////////////////////////////////////////////////////////////////////////////////////////
    // this trait will remove the cv-qualifier, pointer types, reference type and also the array dimension

    template<typename T, typename Enable = void>
    struct raw_array_type  { typedef typename raw_type<T>::type type; };

    template<typename T>
    struct raw_array_type_impl;

    template<typename T, std::size_t N>
    struct raw_array_type_impl<T[N]> { typedef typename raw_array_type<T>::type type; };

    template<typename T>
    struct raw_array_type<T, typename std::enable_if<std::is_array<T>::value>::type> 
    {
        typedef typename raw_array_type_impl<typename remove_cv<T>::type>::type type; 
    };

    template<typename T>
    using raw_array_type_t = typename raw_array_type<T>::type;

    /////////////////////////////////////////////////////////////////////////////////////////

    template <bool... b> struct static_all_of;
    //specialization for type true, go continue recurse if the first argument is true
    template <bool... tail> 
    struct static_all_of<true, tail...> : static_all_of<tail...> {};
    // end recursion if first argument is false
    template <bool... tail> 
    struct static_all_of<false, tail...> : std::false_type {};

    // finish when no argument are left
    template <> struct static_all_of<> : std::true_type {};

    // use it like e.g.:
    // static_all_of<std::is_class<ClassType>::value...>::value

    /////////////////////////////////////////////////////////////////////////////////////////

    template <bool... b> struct static_any_of;

    template <bool... tail> 
    struct static_any_of<true, tail...> : std::true_type {};

    template <bool... tail> 
    struct static_any_of<false, tail...> : static_any_of<tail...> {};

    // finish when no argument are left
    template <> struct static_any_of<> : std::false_type {};

    /////////////////////////////////////////////////////////////////////////////////////////
    /*! 
     * Determine if the given type \a T has the method
     * 'type get_type() const' declared.
     */
    template <typename T>
    class has_get_type_func_impl
    {
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename U, rttr::type (U::*)() const> 
        class check { };

        template <typename C>
        static YesType& f(check<C, &C::get_type>*);

        template <typename C>
        static NoType& f(...);

    public:
        static const bool value = (sizeof(f<typename raw_type<T>::type>(0)) == sizeof(YesType));
    };

    /*!
     * If T has a member function 'type get_type() const;' then inherits from true_type, otherwise inherits from false_type. 
     */
    template<class T, typename Enable = void>
    struct has_get_type_func : std::false_type
    {};

    template<class T>
    struct has_get_type_func<T, typename std::enable_if<has_get_type_func_impl<T>::value>::type > : std::true_type
    {};

    /////////////////////////////////////////////////////////////////////////////////
    
    /*! 
     * Determine if the given type \a T has the method
     * 'type get_type() const' declared.
     */
    template <typename T>
    class has_get_ptr_func_impl
    {
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename U, void* (U::*)()> 
        class check { };

        template <typename C>
        static YesType& f(check<C, &C::get_ptr>*);

        template <typename C>
        static NoType& f(...);

    public:
        static const bool value = (sizeof(f<typename raw_type<T>::type>(0)) == sizeof(YesType));
    };

    /*!
     * If T has a member function 'type get_type() const;' then inherits from true_type, otherwise inherits from false_type. 
     */
    template<class T, typename Enable = void>
    struct has_get_ptr_func : std::false_type
    {};

    template<class T>
    struct has_get_ptr_func<T, typename std::enable_if<has_get_ptr_func_impl<T>::value>::type > : std::true_type
    {};

    /////////////////////////////////////////////////////////////////////////////////
    
    /*! 
     * Determine if the given type \a T has the method
     * 'type get_type() const' declared.
     */
    template <typename T>
    class has_get_derived_info_func_impl
    {
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename U, derived_info (U::*)()> 
        class check { };

        template <typename C>
        static YesType& f(check<C, &C::get_derived_info>*);

        template <typename C>
        static NoType& f(...);

    public:
        static const bool value = (sizeof(f<typename raw_type<T>::type>(0)) == sizeof(YesType));
    };

    /*!
     * If T has a member function 'type get_type() const;' then inherits from true_type, otherwise inherits from false_type. 
     */
    template<class T, typename Enable = void>
    struct has_get_derived_info_func : std::false_type
    {};

    template<class T>
    struct has_get_derived_info_func<T, typename std::enable_if<has_get_derived_info_func_impl<T>::value>::type > : std::true_type
    {};

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

        template<typename T>
    struct get_ptr_impl
    {
        static RTTR_INLINE void* get(T& data)
        {
            return const_cast<void*>(reinterpret_cast<const void*>(&data));
        }
    };

    template<typename T>
    struct get_ptr_impl<T*>
    {
        static RTTR_INLINE void* get(T* data)
        {
            return get_ptr_impl<T>::get(*data);
        }
    };

    template<>
    struct get_ptr_impl<void*>
    {
        static RTTR_INLINE void* get(void* data)
        {
            return data;
        }
    };

    template<>
    struct get_ptr_impl<const void*>
    {
        static RTTR_INLINE void* get(const void* data)
        {
            return const_cast<void*>(data);
        }
    };

    template<typename T>
    static RTTR_INLINE void* get_void_ptr(T* data)
    {
        return get_ptr_impl<T*>::get(data);
    }

    template<typename T>
    static RTTR_INLINE void* get_void_ptr(T& data)
    {
        return get_ptr_impl<T>::get(data);
    }



    
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    template<typename T, typename... Types>
    struct contains : static_any_of<std::is_same<T, Types>::value...>
    {
    };

    template<typename T, typename... Types, template<class...> class TContainer>
    struct contains<T, TContainer<Types...>> : contains<T, Types...>
    {
    };

    /////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct is_array_impl
    {
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename U> static NoType& check(typename U::no_array_type*);
        template <typename U> static YesType& check(...);


        static const bool value = (sizeof(check<array_mapper<T> >(0)) == sizeof(YesType));
    };

    template<typename T>
    struct is_array : std::conditional<is_array_impl<typename remove_cv<typename std::remove_reference<T>::type>::type>::value,
                                       std::true_type,
                                       std::false_type>::type
    {};

    /////////////////////////////////////////////////////////////////////////////////////
    // rank_type<T, size_t>::type
    //
    // rank_type<int[2][10][4], 0>::type => int[2][10][4]
    // rank_type<int[2][10][4], 1>::type => int[10][4]
    // rank_type<int[2][10][4], 2>::type => int[4]
    // works of course with all other classes, which has an array_mapper specialization

    template <typename... T>
    struct concat_array_types;


    template <template <typename ...> class List, typename ...Types, typename T>
    struct concat_array_types<List<Types...>, T, std::true_type>
    {
        using type = List<Types...>;
    };

    template <template <typename... > class List, typename... Types, typename T>
    struct concat_array_types<List<Types...>, T, std::false_type>
    {
        using sub_type = typename array_mapper<T>::sub_type;
        using type = typename concat_array_types< List<Types..., T>, sub_type, typename std::is_same<T, sub_type>::type>::type;
    };
 
    template<typename T>
    struct array_rank_type_list
    {
        using sub_type = typename array_mapper<T>::sub_type;
        using types = typename concat_array_types< std::tuple<>, T, typename std::is_same<T, sub_type>::type>::type;
    };

    template<typename T, size_t N>
    struct rank_type
    {
        using type = typename std::tuple_element<N, typename array_rank_type_list<T>::types>::type;
    };
    
    /////////////////////////////////////////////////////////////////////////////////////
    // rank<T>::value
    //
    // rank<int[2][10][4]>::value => 3
    // rank<std::vector<std::vector<int>>>::value => 2
     template <typename... T>
     struct rank_impl
     {
         using type = typename std::integral_constant<std::size_t, 0>::type;
     };

     template <template <typename... > class List, typename... Types>
     struct rank_impl<List<Types...>>
     {
         using type = typename std::integral_constant<std::size_t, sizeof...(Types) - 1>::type;
     };

    template<typename T>
    using rank = typename rank_impl< typename detail::array_rank_type_list<T>::types >::type;

    /////////////////////////////////////////////////////////////////////////////////////
    // pointer_count<T>::value Returns the number of pointers for a type
    // e.g. pointer_count<char**>::value => 2
    //      pointer_count<char*>::value  => 1
    //      pointer_count<char>::value   => 0
    template<typename T, typename Enable = void>
    struct pointer_count_impl
    {
        static const std::size_t size = 0;
    };

    template<typename T>
    struct pointer_count_impl<T, typename std::enable_if<std::is_pointer<T>::value &&
                                                         !is_function_ptr<T>::value &&
                                                         !std::is_member_pointer<T>::value>::type>
    {
        static const std::size_t size = pointer_count_impl<typename remove_pointer<T>::type>::size + 1;
    };

    template<typename T>
    using pointer_count = std::integral_constant<std::size_t, pointer_count_impl<T>::size>;

    /////////////////////////////////////////////////////////////////////////////////////
    // is_char_array<T>::value Returns true if the given type is a char array
    // e.g. is_char_array<char[10]>::value => true
    //      is_char_array<int[10]>::value => false
    //      is_char_array<char>::value => false
    template<typename T>
    using is_one_dim_char_array = std::integral_constant<bool, std::is_array<T>::value && 
                                                               std::is_same<char, raw_array_type_t<T>>::value &&
                                                               (std::rank<T>::value == 1)>;

    /////////////////////////////////////////////////////////////////////////////////////


    template <typename T, typename... Ts> 
    struct is_type_in_list_impl;

    template <typename T, typename U> 
    struct is_type_in_list_impl<T, U>
    {
        static RTTR_CONSTEXPR_OR_CONST bool value = std::is_same<T, U>::value;
    };

    template<typename T, typename T2, typename... U> 
    struct is_type_in_list_impl<T, T2, U...> 
    {
        static RTTR_CONSTEXPR_OR_CONST bool value = std::is_same<T, T2>::value || is_type_in_list_impl<T, U...>::value;
    };

    template<typename T, class... Ts, template<class...> class List>
    struct is_type_in_list_impl<T, List<Ts...>>
    {
        static RTTR_CONSTEXPR_OR_CONST bool value = is_type_in_list_impl<T, Ts...>::value;
    };

    /////////////////////////////////////////////////////////////////////////////////////
    // Returns true when the type T is contained in a list of template parameters.
    // use it like this:
    // is_type_in_list<int, primitive_list<bool, double, int>>::value => true
    // is_type_in_list<int, primitive_list<bool, double, void>>::value => false
    // or
    // is_type_in_list<int, bool, double, void>::value => false

    template<typename T, typename List>
    using is_type_in_list = std::integral_constant<bool, is_type_in_list_impl<T, List>::value>;
   
    /////////////////////////////////////////////////////////////////////////////////////
    
    template <typename T, typename...Ts> 
    struct max_sizeof_list_impl;

     template <typename T> 
    struct max_sizeof_list_impl<T>
    {
        static RTTR_CONSTEXPR_OR_CONST size_t value = sizeof(T);
    };

    template<typename T1, typename T2, typename... U> 
    struct max_sizeof_list_impl<T1, T2, U...> 
    {
        static RTTR_CONSTEXPR_OR_CONST std::size_t value = max_sizeof_list_impl<
                                                          typename std::conditional<sizeof(T1) >= sizeof(T2), 
                                                                                    T1, T2>::type,
                                                                                    U...>::value;
    };

    template<class... Ts, template<class...> class List>
    struct max_sizeof_list_impl<List<Ts...>>
    {
        static RTTR_CONSTEXPR_OR_CONST std::size_t value = max_sizeof_list_impl<Ts...>::value;
    };

    /////////////////////////////////////////////////////////////////////////////////////
    // Returns the maximum sizeof value from all given types
    // use it like this:
    // max_size_of_list<int, bool, double>::value => 8

    template<typename... Ts>
    using max_sizeof_list = std::integral_constant<std::size_t, max_sizeof_list_impl<Ts...>::value>;


    /////////////////////////////////////////////////////////////////////////////////////
    
    template <typename T, typename...Ts> 
    struct max_alignof_list_impl;

     template <typename T> 
    struct max_alignof_list_impl<T>
    {
        static RTTR_CONSTEXPR_OR_CONST size_t value = std::alignment_of<T>::value;
    };

    template<typename T1, typename T2, typename... U> 
    struct max_alignof_list_impl<T1, T2, U...> 
    {
        static RTTR_CONSTEXPR_OR_CONST std::size_t value = max_alignof_list_impl<
                                                          typename std::conditional<std::alignment_of<T1>::value >= std::alignment_of<T2>::value,
                                                                                    T1, T2>::type,
                                                                                    U...>::value;
    };

    template<class... Ts, template<class...> class List>
    struct max_alignof_list_impl<List<Ts...>>
    {
        static RTTR_CONSTEXPR_OR_CONST std::size_t value = max_alignof_list_impl<Ts...>::value;
    };

    /////////////////////////////////////////////////////////////////////////////////////
    // Returns the maximum sizeof value from all given types
    // use it like this:
    // max_alignof_list<int, bool, double>::value => 8

    template<typename... Ts>
    using max_alignof_list = std::integral_constant<std::size_t, max_alignof_list_impl<Ts...>::value>;

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    // A simple list for types
    // use it like this:
    // using my_list = type_list<int, bool>;

    template<typename... T>
    struct type_list {};

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    using remove_cv_ref_t = typename detail::remove_cv<typename std::remove_reference<T>::type>::type;

    /*!
     * A slightly different decay than in the standard, the extent of arrays are not removed.
     * Applies lvalue-to-rvalue, function-to-pointer implicit conversions to the type T and removes cv-qualifiers.
     */
    template<typename T>
    struct decay 
    {
        typedef typename std::remove_reference<T>::type Tp;
        typedef typename std::conditional<std::is_function<Tp>::value,
                                          typename std::add_pointer<Tp>::type,
                                          typename remove_cv<Tp>::type
                                         >::type type;
    };

    template<typename T>
    using decay_t = typename decay<T>::type;

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    // checks whether the given type T is a unique_ptr or not

    template<typename T>
    struct is_unique_ptr : std::false_type {};

    template<typename T>
    struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};

    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////

} // end namespace detail
} // end namespace rttr

#endif // RTTR_MISC_TYPE_TRAITS_H_