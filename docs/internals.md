ijst 实现时没有使用什么新的技术或黑魔法，甚至也没有使用复杂的模板编程技术，而是通过现有的较常见的技术进行组合。此文档简单记录了这些技术，以及 ijst 的一些设计。

[toc]

# IDL
ijst 不打算通过外部工具生成 C++ 代码，所以需要使用合法的 C++ 语言编写 IDL。在这种情况下，能提供最大自由度的就是宏了。

简单的实现方法是把字段信息重复一遍，如：

```cpp
struct Range {
    int min;
    int max;
    ADD_FIELD((min, max)); // min、max要重复写一遍
};
```

这样很容易出问题。

但是 C++ 不支持反射，无法直接得到结构体的元信息。所以 ijst 选择放弃使用正常的结构体定义方式，使用宏同时定义结构体和记录元信息。具体的 IDL 语法参考了 *Boost::Hana*，并做了简化：

```cpp
IJST_DEFINE_STRUCT (
    struct_name
    , (field_type0, field_name0, "json_name0", filed_desc0)
    // ...
)
```

这基本上就是将所有需要的信息罗列一下，再加上必要的括号和逗号。*（P.S. 后来发现 `BOOST_FUSION_ADAPT_STRUCT` 宏的语法和这完全一样）*
其中：
1. 每个字段的信息外面的括号，是为了实现上方便的考虑。具体而言，是为了减少 `IJST_DEFINE_STRUCT` 宏的参数数量。
2. `json_name` 使用双引号包起来，是因为 JSON key 命名没有 C++ 变量命名时的限制，比如可以以空格开始或结尾，或带有逗号。
3. `field_type` 数据类型不直接使用 C++ 的类型，一是因为不支持所有的 C++ 类型，需避免误用；二是需要与实现隔离。

**宏实参的逗号**

在 `field_type` 中，可能需要使用类似 `map<string, int>` 这样的定义。此类模板声明中的**逗号**，会把这个声明分割成几个参数，即 `map<string` 和 `int>`。这个问题不大好处理。

在简单的宏里，可以使用如下方法绕过：

```cpp
#define COMMA ,
SOME_MACRO(map<string COMMA int>)
```

但是 ijst 中的宏会经过多层替换，无法使用这个技巧。所以需要想办法把逗号用 `()` 包起来。
最直接的想法是把所有的 `field_type` 用括号包起来，然后在使用的时候解包：

```cpp
#define UNPACK(x)   x
#define FIELD(field_type, field_name)  UNPACK field_type field_name
// 使用
FIELD((map<string, int>), m);
```

如此一来就需要将所有的 `filed_type` 定义都加上括号，较为不便。

C++11 后，使用 `decltype` 关键字实现起来不难：

```cpp
#define IJST_TYPE(...)	decltype(__VA_ARGS__())
```

解释一下，还是以上面的 map 为例，宏调用 `IJST_TYPE(map<string, int>)` 展开后是 `decltype(map<string, int>())`，逗号已经被括号包起来了。
`map<string, int>()` 这个表达式的结果是 *prvalue* （参考 [value_category](http://en.cppreference.com/w/cpp/language/value_category)），
而 `decltype` 对 *prvalue* 求值的结果是 *T* （非左值或右值引用，参考 [decltype](http://en.cppreference.com/w/cpp/language/decltype)），即 `map<string, int>` 本身。

C++11 前，则需要更复杂一点，需要借助模板的 Function Type（参考 [StackOverFlow](https://stackoverflow.com/a/13842784)）：

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

**MSVC 中的 __VA_ARGS__**

下面这段代码：

```cpp
#define PLUS(a1, a2) ((a1)+(a2))
#define VA(...) PLUS(__VA_ARGS__)
VA(1,2）
```

在正常情况下 `PLUS(__VA_ARGS__)` 会把参数解包，替换的结果应该是 `((1)+(2))`。
但在 MSVC 中， `PLUS` 会把 `__VA_ARGS__` 当成一个参数传递，得到的结果是 `((1,2)+())`。
MSVC 认为这是正确的结果，并不打算修改（参考 [StackOverflow](https://stackoverflow.com/a/7459803)），所以代码中需要对此进行兼容：

```cpp
#define IJSTI_EXPAND(...) __VA_ARGS__
#define VA(...) IJSTI_EXPAND(PLUS(__VA_ARGS__))
```

# 不定参数的宏

接下来，就是按照这样的宏语法生成代码。

很明显，每个 ijst 结构体的字段数量是不定的，即调用 `IJST_DEFINE_STRUCT` 宏的实参个数是不定的。宏本身支持不定参数，但是宏不支持递归或循环，所以不能直接使用这些参数。

解决方法很暴力：分别实现每个参数数量的宏，然后计算出实参的数量，再调用对应的宏。

计算宏参数数量的方法很常见：

```cpp
// 获取 IJST_DEFINE_STRUCT 宏中 field 的数量
// 调用宏时必须声明 stName，即宏至少有一个参数
#define IJSTI_PP_NFIELD(...) \
    IJSTI_PP_NFIELD_IMPL(__VA_ARGS__ \
     ,64 ,63 ,62 ,61 ,60 , /*...*/ ,3 ,2 ,1 ,0)
#define IJSTI_PP_NFIELD_IMPL( \
   e0 , e1 , e2 , e3 , e4 , /*...*/ , e60 , e61 , e62 , e63 , e64, \
   N, ...) N
```

实现每个参数数量的宏，再根据宏参数的数量进行调用：

```cpp
// 字段为 0 个时调用以下宏：
#define IJSTI_DEFINE_STRUCT_IMPL_0(stName)          //...
// 字段为 1 个时调用以下宏：
#define IJSTI_DEFINE_STRUCT_IMPL_1(stName, f0)      //...
// 字段为 2 个时调用以下宏：
#define IJSTI_DEFINE_STRUCT_IMPL_2(stName, f1, f2)  //...
//...
```

这些宏的实际定义是固定的，可以接受使用代码生成器。生成的方法很多，自己写个程序也不麻烦。为了尽量少的依赖外部工具，且可以使用强大的 *boost PP (boost.preprocessor)* 库，所以 ijst 选择使用 C++ 预处理器作为代码生成器，即使用宏生成宏。

方法简单来说，就是使用 `BOOST_PP_ITERATE` 触发每个参数数量的宏的生成，然后使用 `BOOST_PP_REPEAT` 和 `BOOST_PP_ENUM_PARAMS` 等宏生成每个宏的具体实现。

一般而言，通过 *boost PP* 宏生成的是普通的 C++ 代码，但 ijst 需要生成的是一系列的**宏定义**，即 `#define` 这样的语句。在输入代码中直接写 `#define`（或加上空格），会被预处理器当成普通的宏定义处理，无法输出。这时需要间接生成 `#define` 语句：

```cpp
#define IJSTM_HASH 	#   // 无法在 C++ 中直接使用
IJSTM_HASH  define      // 生成 "# define"
```

另外，可以尽量将宏定义的内容提取出来，以减小生成文件的体积。

具体可参考 `ijst/detail/ijst_repeat_def_src.h`，代码量不多。

# 元信息注册及获取

C++ 不支持反射，需要主动注册元信息。元信息的一个目的是通过*类型*得到所需要的信息，即 map，注册的时候往这个 map 里写入信息，读取的时候从里面获取信息。直接的想法是使用 `map<type_index(type_id(Type)), MetaClassInfo>` 储存这个映射关系，但这样实现单例模式会有不便，且很难保证在元信息会在使用前被插入。
比如常用的做法在一个全局变量初始化的过程中完成元信息的写入，但这在使用 static library 时可能会发生问题，参考 [StackOverflow](https://stackoverflow.com/questions/9459980/c-global-variable-not-initialized-when-linked-through-static-libraries-but-ok)。

ijst 的选择是使用**模板**，实现类型与元信息的映射以及单例：

```cpp
// T 为元信息的类型
template<typename T>
struct MetaClassInfoTyped {
    MetaClassInfo info;
};

class Singleton<MetaClassInfoTyped<Type> >;  // 或直接把 MetaClassInfoTyped 实现为单例模式
```

这样使用单例的 `GetInstance` 方法即可获取元信息，也避免了 map 的查找操作。

接下来是元信息的初始化。首先考虑注册代码的**生成**，在 `IJST_DEFINE_STRUCT` 中生成：

```
#define IJST_DEFINE_STRUCT_IMPL_1(stName, f0) \
    class stName {  /* 类定义 */ };  \
    //... 相关的元信息注册代码
```

接下来需要考虑的是，怎么样让这些代码在合适的时候**运行**起来。直接的想法是借助单例模式，将这些代码在 `MetaClassInfoTyped` 的构造函数中定义：

```cpp
#define IJST_DEFINE_STRUCT_IMPL_1(stName, f0) \
    class stName {  /* 类定义 */ };  \
    template<> MetaClassInfoType<stName>::MetaClassInfoTyped()  \
    { /* 元信息的注册 */ }
```

这样，单例在初始化的时候，就会自动运行生成的代码。

**但是**，C++ 有一个限制，是模板必须在相同的命名空间里特化。如果在自定义的命名空间里使用 `IJST_DEFINE_STRUCT` 宏的话，会引发编译错误。所以不能直接生成特化代码：

```cpp
template<typename T>
struct MetaClassInfoTyped {
    MetaClassInfoTyped()
    {
        // 调用生成的函数
        T::InitMetaInfo(this);
    }
};

#define IJST_DEFINE_STRUCT_IMPL_1(stName, f0)  \
    class stName {  \
        /* 正常的类定义 */  \
        void InitMetaInfo(MetaClassInfoTyped* p)  \
        {  \
            /* 元信息的注册 */  \
            /* 注意此时 MetaClassInfoTyped 还没构造完成，不能调用其单例的 GetInstance() 方法 */  \
        }  \
    };
```

# 元信息内容

必要的元信息包括字段的类型以及定位方法，加上业务所需的其他信息。具体到 ijst，业务信息包括字段对应的 JSON 键名、描述（如 `ijst::Optional` ）、相关操作代码的函数地址。

## 类型及定位方法

作为一个静态类型语言，无法通过类型信息在运行期动态地创建该类型的变量，其更多的作用是作为某个 map 的 key。和前面一样， ijst 选择使用**模板特化**来达到这个目的，并不在运行期储存和使用类型信息。

对于定位方法，C++ 有一个强大的工具来保存一个变量的访问方法：指针。实际上，ijst 保存的是字段在结构体内的偏移。在使用的时候，通过结构体的指针加上偏移，即可得到该字段的指针。C++ 提供了 `offsetof` 宏来获取这个偏移，但是它最大的缺点是要求结构体必须是 *standard-layout* 的，而常用的 `std::vector` 和 `std::map` 都无法保证满足这个条件。
ijst 虽然提供了 `T_Wrapper` 类，以保存指针的方式，将任意类型包装为一个 *standard-layout* 的类型。但是这样在使用便利和运行效率上有会受到一定的影响。

最终 ijst 在这里放弃符合 C++ 标准，使用零指针偏移的方法取得这个偏移量：

```cpp
#define IJST_OFFSETOF(T, member)    ((size_t)&(((T*)0)->member))
```

这是一个常见的做法，但是这是有一定问题的。首先是 `T.member` 可能会重载 `&` 操作符。但是因为需要进行操作的都是 ijst 预定义的类型，可以避免这个问题。另一个问题是在标准中，这种方法无法保证避免零指针的解引用。但是在各个编译环境的测试中，都可以获取到正确的偏移，所以使用了这种方法。

## 业务信息
业务信息中的 JSON 键名、描述等都可以直接从 IDL 中获取。但是相关的操作（序列化/反序列化等），需要自行获取其函数地址。为便于代码生成，采用模板特化的方法实现这些方法：

```cpp
// 为便于编码，使用多态。实际也可以直接保存函数地址
class SerializerInterface {
    // 接口
};

// 模板定义，这个通用的模版不应该被实例化
template<typename T, typename Enable = void>
class FSerializer : public SerializerInterface {
};

// 对于每种类型，编写相关的序列化/反序列化代码
template<> class FSerializer<T_int> : public SerializerInterface {
    // 实现
};

// 获取和储存对象地址，FIELD_TYPE 是 IDL 中声明的数据类型
SerializerInterface& intf = Singleton<FSerializer<FIELD_TYPE> >::GetInstance();

// 注：FIELD_TYPE 是不能提前完全确定的，比如 vector<T_int> 或 vector<vector<T_int> > 都是支持的，所以不能使用宏拼接函数名
// 如： SerializerInterface& intf = FSerializer##TYPE::GetInstance();
```

**SFINAE**

`FSerializer` 中除了 `T`，还有另外一个类型参数 `Enable`，这是为了使用 SFINAE 技术。因为 ijst 支持结构体的嵌套，所以需要针对“ijst 结构体”这种类型进行特化。

如果不使用 SFINAE，可以这样实现：

```cpp
// Type tags:
template<typename T> struct Primitive<T> {};
template<typename T> struct IjstStruct<T> {};

// IDL 接口中的数据类型
#define T_int   Primitive<int>
#define T_ST(T) IjstStruct<T>

// FSerializer 实现
template<> class FSerializer<Primitive<int> > : public SerializerInterface
{ typedef int VarType; /*...*/ };

template<typename T> class FSerializer<IjstStruct<T> > : public SerializerInterface
{ typedef T VarType; /*...*/ };

// 声明 IDL 对应的数据类型，FIELD_TYPE 是 IDL 中声明的数据类型
FSerialize<FIELD_TYPE>::VarType var;
```

这样会增加模板的数量，而且会在定义结构体字段时依赖 `FSerializer`。

使用 SFINAE 的方案如下：

```cpp
template <typename T>
struct IfHasType { typedef void Void; };

// IDL 接口中的数据类型
#define T_int   int
#define T_ST(T) T

// FSerializer 实现
template<> class FSerializer<int> : public SerializerInterface { /*...*/};

template<typename T>        // ijst 结构体都会定义 _ijst_AccessorType
class FSerializer<T, typename IfHasType<typename T::_ijst_AccessorType>::Void>: public SerializerInterface { /*...*/ };

// 声明 IDL 对应的数据类型，FIELD_TYPE 是 IDL 中声明的数据类型
FIELD_TYPE var;
```

该 SFINAE 中，使用一个类型是否有 `_ijst_AccessorType` 的类型定义判断其是否为 ijst 结构体。

另外一个方法是根据类型中是否有类型为 `Accessor`，名字为 `_` 的成员判断：

```cpp
template <typename T, Accessor<void> T::*>
struct IfIsAccessor {
	typedef void Void;
};

template<typename T> 
class FSerializer<T, typename IfIsAccessor<T, T::_>::Void>: public SerializerInterface { /*...*/ };
```

另外，在实现 `ijst::Optional` 时也需要使用 SFINAE 判断类型是否为 ijst 结构体。

# 序列化/反序列化

在实现时这部分时，由于对 JSON 库了解得不够，曾经走了一些奇怪的弯路。现在留下的实现，使用的都是比较直接的思路：底层库是 RapidJSON，序列化时使用 SAX API，反序列化时使用 DOM API。

为什么反序列化时不用 DOM API？原因如下：
1. 太难实现了。
2. 目前反序列化的性能瓶颈不是由 DOM API 引起的。
3. 若使用 SAX API，在反序列化 vector 时，无法保证扩容时元素能用 move 的方式移动，无法保证效率能比 DOM API 更高。

在这个基础上，提供 API 时尽量能暴露出 RapidJSON 所有的功能，比如 `ParseFlags` 等。
但是目前 ijst 只能处理 UTF8 的 JSON，这在未来需要改进。

# Getter Chaining

有时在访问深层路径的元素时，只需关注最终能否得到结果，而不必关心中间步骤。但是在一般情况下，需要在访问路径的每个步骤都加入判断，较为繁琐。ijst 提供了 Getter Chaining 以解决这个问题。

Getter Chaining 的模式为： `STRUCT OP F1 OP F2 OP F3 ...`，其中 `Fn` 可以用来表示某个成员。因为 OP 必须处于两个操作数的中间，所以无法使用宏，而只能使用类成员函数或者操作符重载。另外，这里的目标是提供一个静态类型的操作，每步 OP 返回的结构应该都是有具体类型的，所以需要使用模板。

ijst 做了一些尝试：

```cpp
template<typename T>
Optional<T> GetOpt(T& field);
// 这样无法 chaining ：st._.GetOpt(st.v1)->_.GetOpt(???.v2)

template<typename T>
Optional<T> GetOpt2(size_t offset);
// 这样使用很麻烦： st._GetOpt2<int>(OFFSET_OF(st, v))->_.GetOpt2<string>(OFFSET_OF(st2, v));
```

现在一直没有找到别的比较好的办法，所以 ijst 对每个字段生成一个 `get_` 方法。

另外， `Optional` 需要根据不同类型提供不同操作符重载：
1. vector<T> 类型：重载 `Optional<T> operator [](size_type i)`，内部值为 nullptr 或 `i` 不存在时返回 `Optional<T>(nullptr)`。
2. map<string, T> 类型：重载 `Optional<T> operator [](string key)`，内部值为 nullptr 或 `key` 不存在时返回 `Optional<T>(nullptr)`。
3. ijst 结构体类型（通过 SFINAE 判断，假设类型为 Tijst ）：重载 `Tijst* operator ->()`，内部值为 nullptr 时，返回 `&Tijst(false)`。

其中，在 `Tijst(false)` 表示该 ijst 结构体对象是个无效的对象。另外一个选择是直接返回 nullptr，然后在 ijst 的 `get_*` 方法中，对 `this` 进行判断，也能达到效果：

```cpp
struct SampleType {
    int v1;
    // 生成的 get_ 函数
    Optional<int> get_v1() {
        if (this == nullptr) return Optional<int>(nullptr);
        else ...
    }
}
```
在大多数的编程器实现中，是可以通过 nullptr 的类指针变量调用其成员函数的。但是 C++ 标准并未对此做出保证，而且 `this == nullptr` 这个判断可能会引发编译器警告。


# 小优化

## 减少 new 操作
ijst 的目标不是一个性能很高的库，但性能也不能太差。

实现完大体功能后进行 benchmark 时，发现反序列化性能和 RapidJSON 有非常大的差距（90 ms vs. 10 ms）。Profile 后发现，`new` 和 `delete` 操作的独占时间比较高，所以尝试减少不必要的 `new` 操作。

在 `FSerializer::FromJson()`中，会根据是否需要返回错误信息，动态 new `Document` 对象。将这部分逻辑改成在栈上使用 `Document` 对象后，反序列化的耗时降至约 38 ms，其中 *canada.json* 这个测试用例的耗时从 54 ms 降至 12 ms。

在 `Accessor` 的构造函数中，有三次 `new` 操作。将其整合成一次后，耗时降低 3 ms，即 35 ms。合并的方法是将这几个需要 new 的对象放入结构体：

```cpp
struct Resource {
    rapidjson::Value unknown;
    rapidjson::Document ownDoc;
    FieldStatusType fieldStatus;
};
Resource* m_r;

// operator new 分配空间
m_r = static_cast<Resource *>(operator new(sizeof(Resource)));

// placement new 完成构造
new(&m_r->fieldStatus) FieldStatusType(m_pMetaClass->GetFieldsInfo().size(), FStatus::kMissing);
new(&m_r->unknown)rapidjson::Value(rapidjson::kObjectType);
new(&m_r->ownDoc) rapidjson::Document();
```

另外，在 `Accessor::CheckFieldStatus()` 中，有一个未使用的 `std::stringstream` 变量，删去后耗时降低 5 ms，即 30 ms（原因不明）。

## 使用简单的数据结构
ijst 中，存在需要许多使用 map 的场景，如通过 JSON 键名查字段元信息，通过字段 offset 查找字段的状态等。最开始使用 `std::map` 储存这些对应关系，后来发现内存占用较高。后来改为使用数组 + 二分查找的方法，在不降低效率的同时，将内存占用减少了 1/3 ~ 1/2。

## Extern template
作为一个纯头文件、且大量使用模板的库，编译速度是一个值得注意的问题。
一方面，在开发的时候需要照顾编译器的感受，减少模板的使用（现在通过gcc `-ftime-report` 功能测得编译时模板实例化的时间占比约 20%）。
另一方面，可以使用 C++11 的 extern template 功能避免在多个 cpp 文件中**重复编译**。

其中，ijst 使用的 RapidJSON 相关的模板类，如 `Value`，`MemoryPoolAllocator<>` 等可以很容易地使用此类方法：

```
#if IJST_EXTERN_TEMPLATE
// 不要实例化这些模板
extern template class GenericValue<UTF8<> >;
extern template class MemoryPoolAllocator<>;
#endif

#if IJST_EXPLICIT_TEMPLATE
// 显式实例化
template class GenericValue<UTF8<> >;
template class MemoryPoolAllocator<>;
#endif
```

另外，ijst 本身也有代码量较大的模板，如 `FSerialize`。但是每个 ijst 结构体会使用不同的参数实例化 `FSerialize`，而且很难控制不对同一个类型的 `FSerializer` 进行多次显示实例化（这会引起编译错误）。所以不能**直接**控制 `FSerializer` 的实例化。

ijst 的方法是尽量收归会引起 `FSerializer` 实例化的地方。收归后仅会在 ijst 结构体的 `_ijst_InitMetaInfo()` 及其调用的函数中引起 `FSerializer` 的实例化。所以只需控制 `_ijst_InitMetaInfo()` 方法的实例化即可。为此，需要将这个函数用模板实现（添加一个无用的模板参数即可）。

另外一个代码量较大的类是 `Accessor`，和上面一样，需要使用模板实现这个类。

开发过程中，可使用 objdump 工具检查模板是否被实例化。

**XMacro**

可以注意到，extern template 和模板显式实例化声明时的模板总是相同的。这里可以使用 XMacro 减少重复代码量，并降低出错的可能：

```cpp
#define IJSTI_EXTERNAL_TEMPLATE_XLIST \
		IJSTX(class rapidjson::GenericValue<rapidjson::UTF8<> >) \
		IJSTX(class rapidjson::MemoryPoolAllocator<>)

#if IJST_EXTERN_TEMPLATE
	#define IJSTX(...)	extern template __VA_ARGS__;
	IJSTI_EXTERNAL_TEMPLATE_XLIST
	#undef IJSTX
#endif

#if IJST_EXPLICIT_TEMPLATE
	#define IJSTX(...)	template __VA_ARGS__;
	IJSTI_EXTERNAL_TEMPLATE_XLIST
	#undef IJSTX
#endif
```
