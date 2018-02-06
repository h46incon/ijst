# 元信息

## IDL
ijst 不打算通过外部工具生成 C++ 代码，所以需要使用合法的 C++ 语言编写 IDL。在这种情况下，能提供最大自由度的就是宏了。

另一个限制条件，就是不要把字段信息重复一遍，比如：

```cpp
struct Range {
    int min;
    int max;
    ADD_FIELD((min, max)); // min、max要重复写一遍
};
```

这样很容易出问题。C++ 不支持反射，无法直接得到结构体的元信息。所以 ijst 选择放弃使用正常的结构体定义方式，而使用宏直接定义结构体。参考 `Boost::Hana`，设计了下面的 IDL：

```cpp
IJST_DEFINE_STRUCT (
    struct_name
    , (field_type0, field_name0, "json_name0", filed_desc0)
    // ...
)
```

这基本上就是将所有需要的信息罗列一下，再加上必要的括号和逗号。
其中：
1. 每个字段的信息外面的括号，是为了实现上方便的考虑。具体而言，是为了减少 `IJST_DEFINE_STRUCT` 宏的参数数量。
2. json_name 使用双引号包起来，是因为 JSON key 命名没有 C++ 变量命名时的限制，比如可以以空格开始或结尾，或带有逗号。
3. field_type 数据类型不直接使用 C++ 类型，一是因为不支持所有的 C++ 类型，二是需要与实现隔离。

### 宏实参的逗号
在 field_type 中，可能需要使用类似 `map<string, int>` 这样的定义。此类模板声明中的逗号，会把这个声明分割成几个参数，即 `map<string` 和 `int>`。
这个问题不大好处理。在简单的宏里，可以使用如下方法绕过：

```cpp
#define COMMA ,
SOME_MACRO(map<string COMMA int>)
```

但是 ijst 中的宏会经过多层替换，无法使用这个技巧。所以需要想办法把逗号用 `()` 包起来。
最直接的想法是把所有的 field_type 用括号包起来，然后在使用的时候解包：

```cpp
#define UNPACK(x)   x
#define SOME_MACRO(field_type, field_name)  UNPACK field_type field_name
// 使用
SOME_MACRO((map<string, int>), m);
```

如此一来就需要将所有的 filed_type 定义都加上括号，较为不便。如果能达到普通类型，按正常写法，带逗号的类型则按某种特殊方法，也可以接受。

C++11 后，使用 `decltype` 关键字实现起来不难：

```cpp
#define IJST_TYPE(...)	decltype(__VA_ARGS__())
```

还是以上面的 map 为例，宏调用 `IJST_TYPE(map<string, int>)` 展开后是 `decltype(map<string, int>())`，逗号已经被括号包起来了。
`map<string, int>()` 这个表达式的结果是 *prvalue* （参考 [value_category](http://en.cppreference.com/w/cpp/language/value_category)），
而 `decltype` 对 *prvalue* 求值的结果是 *T* （非左值或右值引用，参考 [decltype](http://en.cppreference.com/w/cpp/language/decltype)），即 `map<string, int>` 本身。

C++11 前，则需要更复杂一点，需要借助模板的 Function type（[参考](https://stackoverflow.com/a/13842784)）：

```cpp
template<typename T>
struct ArgumentType;

template<typename T, typename U>
struct ArgumentType<U(T)> {typedef T type;};


#define IJST_TYPE(...)	ArgumentType<void(__VA_ARGS__)>::type
```

这样，`IJST_TYPE(map<string, int>)` 会被替换为 `ArgumentType<void(map<string, int>)>::type`，即 `map<string, int>`。

一句题外话，C++ template 的尖括号语法也引发了一些问题：有些情况下，无法确定尖括号是模板的参数列表，还是大于小于比较符。所以有时需要写出这样的代码：

```cpp
ClassA::template Foo<0>(1);
ClassA a;
a.template Bar<2>(3)
```

接下来，就是使用这样的宏定义生成代码。

## 不定参数的宏

宏本身支持不定参数，但是宏不支持递归或循环，所以不能直接使用这些参数。
解决方法很暴力：计算出参数的数量，并对每个数量分别实现宏。

获取宏数量的方法很常见：

```cpp
// 获取 IJST_DEFINE_STRUCT 宏中 field 的数量
#define IJSTI_PP_NFIELD(...) \
    IJSTI_PP_NFIELD_IMPL(__VA_ARGS__ \
     ,64 ,63 ,62 ,61 ,60 , /*...*/ ,3 ,2 ,1 ,0 \
    )
#define IJSTI_PP_NFIELD_IMPL( \
   e0 , e1 , e2 , e3 , e4 , /*...*/ , e60 , e61 , e62 , e63 , e64, \
   N, ...) N
```

获取参数数量之后，就需要对每个数量进行实现了：

```cpp
#define IJSTI_DEFINE_STRUCT_IMPL_0(stName)          //...
#define IJSTI_DEFINE_STRUCT_IMPL_1(stName, f0)      //...
#define IJSTI_DEFINE_STRUCT_IMPL_2(stName, f1, f2)  //...
//...
```

这些定义是静态的，可以接受使用代码生成器。生成的方法很多，自己写个程序也不麻烦。
为了尽量少的依赖外部工具，且 boost::preprocessor 也很强大，所以这里选择使用 C++ 预处理器作为代码生成器，即使用宏生成宏。

方法简单来说，就是使用 `BOOST_PP_ITERATE` 触发每个参数数量的宏的生成，然后借助 `BOOST_PP_REPEAT` 和 `BOOST_PP_ENUM_PARAMS` 等宏生成每个宏的具体实现中的重复部分。

使用 boost::preprocessor 达到这个目的比较容易，但有些地方需要稍微绕一下。
因为我们的目标代码是一系列的宏定义，即 `#define` 这样的语句。但在源代码中直接写 `#define`，或加上空格，都会被预处理器当成普通的宏定义处理。这时需要间接生成 `#define` 语句：

```cpp
#define IJSTM_HASH 		#
IJSTM_HASH  define      // 生成 "# define"
```

另外，可以尽量将宏定义的内容提取出来，以减小生成文件的体积。

具体可参考 `ijst/detail/ijst_repeat_def_src.h`，代码量不多。

## 元信息注册

C++ 不支持反射，所以只能主动注册元信息。元信息套路，就是通过*类型*得到一个 map，注册的时候往这个 map 里写入信息，读取的时候从里面获取信息。
即元信息是和类型对应的。直接的想法是使用 `map<type_index(type_id(Type)), MetaClassInfo>` 储存这个映射关系，但这样实现单例模式会有不便。

ijst 的选择是使用模板，实现类型与元信息的映射以及单例：

```cpp
template<typename T>
struct MetaClassInfoTyped {
    MetaClassInfo info;
};

class Singleton<MetaClassInfoTyped<Type> >;  // 或直接把 MetaClassInfoTyped 实现为单例模式
```

这样元信息的获取很简单，也避免了 map 的查找操作。

关于元信息的写入，首先是考虑注册代码的生成。这步比较简单，也没什么选择，让 `IJST_DEFINE_STRUCT` 生成类定义的时候生成：

```
#define IJST_DEFINE_STRUCT_IMPL_1(stName, f0) \
    class stName {  /* 类定义 */ };  \
    //... 相关的元信息注册代码
```

接下来需要考虑的是，怎么样让这些代码在合适的时候**运行**起来。直接的想法就是，借助单例模式，将这些代码在 `MetaClassInfoTyped` 的构造函数中定义即可。
结合模板特化，这很容易做到：

```cpp
#define IJST_DEFINE_STRUCT_IMPL_1(stName, f0) \
    class stName {  /* 类定义 */ };  \
    template<> MetaClassInfoType<stName>::MetaClassInfoTyped()  \
    { /* 元信息的注册 */ }
```

这样，单例在初始化的时候，就会自动运行生成的代码。

**但是**，C++ 有一个限制，是模板必须在相同的命名空间里特化。所以如果在自定义的命名空间里使用 `IJST_DEFINE_STRUCT` 宏的话，会引发编译错误。
所以需要绕过一下，在宏里面定义一个函数，然后在 `MetaClassInfoTyped` 的构造函数调用这个函数：

```cpp
template<typename T>
struct MetaClassInfoTyped {
    MetaClassInfoTyped()
    {
        T::InitMetaInfo(this);
    }
};

#define IJST_DEFINE_STRUCT_IMPL_1(stName, f0)  \
    class stName {  \
        /* 正常的类定义 */  \
        void InitMetaInfo(MetaClassInfoTyped* p)  \
        {  \
            /* 元信息的注册 */  \
            /* 注意此时 MetaClassInfoTyped 还没构造完成，不能调用其单例的 GetInstance() 方法，否则会导致无限递归 */  \
        }  \
    };
```
### 关于单例

单例模式有很多种实现方法，但是 C++03 还没有标准的锁，所以只能借助静态变量实现单例模式。
虽然模板的静态成员可以在头文件中定义，但这样会导致饿汉初始化。而使用函数内的静态变量则会灵活一些。
需要注意的是，C++11 之前函数静态变量的初始化是不保证线程安全的，这种情况下需要借助类静态变量（TODO：模板类的静态变量在什么时候初始化？）：

```cpp
template<typename T>
class Singleton {
public:
    static T& GetInstance()
    {
        static T ins;   // C++11 前非线程安全
        return ins;
    }

	inline static void GlobalAccess()
	{
		volatile void* dummy = initInstanceTag;     // 使用 volatile 变量避免代码被优化掉
		(void)dummy;
	}
private:
	static void* initInstanceTag;
}
//模板的静态成员可以在头文件中定义
template<typename T> void *Singleton<T>::initInstanceTag = &Singleton<_T>::GetInstance();
```

一般情况下，`GetInstance()` 内的静态变量会在第一次访问该函数的时候初始化。
另外，C++ 模板的成员仅在有访问的时候才会实例化。所以一般情况下，`initInstanceTag` 是不会被实例化的。
但是一旦有代码访问了 `GlobalAccess()`，就会导致它的实例化。而它在初始化的时候，就会访问 `GetInstance()` 函数，引发单例的初始化。
//TODO: 一般的静态变量不保证能被初始化，但模板这种定义在头文件的内

## MS_VC，逗号, use tech in xmacro?
## 单例、 InitBeforeMain
## Stangdard layout

# Getter Chaining
## Optional
## NULL pointer deference, this check null warning

# 其他
## SFINAE
## constexpr if
## ArugmentType
## Batch new
## Allocator，怎么释放的坑
## Unknown reference
## X Macro
## Extern template