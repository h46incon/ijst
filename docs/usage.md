[TOC]

# 定义结构体

使用 ijst 唯一额外的工作就是使用 `IJST_DEFINE_STRUCT` 或 `IJST_DEFINE_STRUCT_WITH_GETTER` 宏定义 ijst struct：

```cpp
IJST_DEFINE_STRUCT (
    struct_name
    , (field_type0, field_name0, "json_name0", filed_desc0)
    , (field_type1, field_name1, "json_name1", filed_desc1)
    // ...
)

// IJST_DEFINE_STRUCT_WITH_GETTER 宏用法同上
```

通过该宏可以指定结构体的名字和每个字段的信息。

一个例子：

```cpp
#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <ijst/types_container.h>
using namespace ijst;

IJST_DEFINE_STRUCT (
    SampleStruct
    , (T_int, iID, "id", 0)
    , (T_string, strName, "name", 0)
    , (T_bool, bSex, "sex", 0)     // 只是举一个bool的栗子
    // 接下来是复杂的字段
    , (IJST_TVEC(T_uint64), vecFriendsID, "friends_id", ijst::FDesc::Optional)      // Optional，可能没朋友
    , (IJST_TMAP(IJST_TVEC(T_string)), mapWhatEver, "what_ever", ijst::FDesc::NotDefault)    // NotDefault 表示这个 map 不为默认值，即不为空
);

// 定义该类型的变量
SampleStruct sampleStruct;
```

宏中每个参数的定义如下：

- **StructName**

    结构体名字，没有特殊的限制。

- **FieldType**

    字段类型。该类型需要是 ijst 预定义的类型。

    目前已实现的类型如下：

    - **原子类型**

        在 `ijst/types_std.h` 中定义。提供的类型有 `T_int, T_int64, T_uint, T_uint64, T_string, T_raw, T_bool, T_ubool, T_wbool`。

        由于 `std::vector<bool>` 的特殊性， ijst 提供了 `T_bool(bool), T_ubool(uint8_t), T_wbool(BoolWrapper)` 这几种类型来储存 bool 变量。

        如果不能确定某个字段的类型，则可以使用 `T_raw` 类型操作原始的 `rapidjson::Value` 对象。

    - **容器类型**

        在 `ijst/ijst.h` 中定义。提供的宏为 `IJST_TVEC(T), IJST_TDEQUE(T), IJST_TLIST(T)，IJST_TMAP(T)，IJST_TOBJ(T)`。

        ijst 分别用以下宏表达 JSON 的 list：

        - `IJST_TVEC(T)`： 将 list 序列化为 `std::vector<T>`。
        - `IJST_TDEQUE(T)`： 将 list 序列化为 `std::deque<T>`。
        - `IJST_TLIST(T)`： 将 list 序列化为 `std::list<T>`。

        用以下宏表达非固定键的 object：

        - `IJST_TMAP(T)`： 将 object 序列化为 `std::map<std::string, T>`。
        - `IJST_TOBJ(T)`： 将 object 序列化为 `std::vector<ijst::T_Member<T> >`，即以数组的形式储存键值对。

        容器的元素类型可以为原子类型和容器（即支持**嵌套**定义）。
        如 `IJST_TVEC(T_int)` 可表达 JSON 值 *[1, 2, 3]*， `IJST_TMAP(IJST_TVEC(T_ubool))` 可表达 JSON 值 *{"key1": [true, true], "key2": [true, false]}*。

    - **ijst 结构体类型**

        在 `ijst/ijst.h` 中定义。提供的宏为 `IJST_TST(T)`。

        可使用该宏在结构体中包含其他的结构体，如 `IJST_TST(SampleStruct)`。

    如有特殊需求，可参考代码自行添加类型。仅需实现相应的序列化\反序列接口(`ijst::detail::SerializeInterface`)即可。

- **FieldName**

    成员变量名。

    变量名不能为 `_`（一个下划线）。如果使用 `IJST_DEFINE_STRUCT_WITH_GETTER`，变量名也不能为 `get_其他变量名`。

    不要使用 `_ijst` 开头的变量名，这可能会和 ijst 的内部实现产生冲突。

- **JsonName**

    字段对应的 JSON key。

- **FieldDesc**

    字段描述。该值可以为`ijst::FDesc`中值的组合，如无特殊情况，使用0即可。值的含义如下：

    - Optional：该字段在 JSON 中不必须出现。
    - Nullable：该字段的 JSON 值可能为 null。
    - NotDefault：该字段不能为默认值，如数组不能为空，`T_int` 值不能为0。


# 接口

在生成一个 ijst 结构体代码，除了用户定义的字段，还会添加一个名为 `_` 的 `Accessor` 类型成员。 通过该成员可以完成对象的序列化、反序列化等操作。

如可以将结构体和 JSON 字符串间转换：

```cpp
SampleStruct sampleStruct;
int ret;

// 序列化
std::string strOut;
ret = sampleStruct._.Serialize(strOut);

// 反序列化
std::string strJson;
//... Init strJson
ret = sampleStruct._.Deserialize(strJson);

// 访问 Unknown 字段
rapidjson::Value& jUnknown = sampleStruct._.GetUnknown();
```

另外，也可以从 `rapidjson::Value` 反序列化：

```cpp
SampleStruct sampleStruct;
int ret;

// FromJson
rapidjson::Value jVal;
ret = sampleStruct._.FromJson(jVal);

```

这些例子中省略了一些默认参数。可通过这些参数指定序列化/反序列化时的具体行为。API 的默认参数提供最不容易出错的行为，但可能会引起一些额外的性能损耗。
完整的接口定义请参考 [Doxygen/html](Doxygen/html)，或直接阅读源码中的函数说明。但是在阅读 API 文档前，建议继续往下阅读以得到大致的了解。


# 字段的状态

ijst 会记录每个字段的状态（在 `ijst::FStatus` 中定义），这些状态会影响**序列化**时的行为。可能的状态如下：

- kValid：已设置为有效值。按实际值序列化。
- kMissing：未设置有效值。如序列化时启用 `SerFlag::kIgnoreMissing` 选项，则不参与序列化。
- kNull：值为 null。序列化时值为 null。如序列化时启用 `SerFlag::kIgnoreNull` 选项，则不参与序列化。

ijst 初始化时，所有字段都是 `kMissing` 状态。可通过相关宏获取或改变其状态：

```cpp
// 获取状态
IJST_GET_STATUS  (obj, field)
// 设置状态
IJST_MARK_VALID  (obj, field)
IJST_MARK_NULL   (obj, field)
IJST_MARK_MISSING(obj, field)
```

注意，直接修改字段的值不会改变其状态。除了通过上述的宏外，也可通过 `IJST_SET` 相关的宏改变字段的值，同时将其设置为 `kValid` 状态：

```cpp
IJST_SET(obj, field, val)
IJST_SET_STRICT(obj, field, val)    // field 和 val 的类型必须完全相同
```

一个例子：

```cpp
SampleStruct st;
st.iID = 42;
// 此时 iID 还是 missing 状态
assert( IJST_GET_STATUS(st, iID) == ijst::FStatus::kMissing );

// 需手动标记改其状态为 valid
IJST_MARK_VALID(st, iID);
assert( IJST_GET_STATUS(st, iID) == ijst::FStatus::kValid );

// 或使用 SET 宏在设置字段值的同时，将其状态设置为 valid
IJST_SET(st, strName, "unique name");
assert( IJST_GET_STATUS(st, strName) == ijst::FStatus::kValid );
```


# Unknown 字段

## (反)序列化时的行为

在反序列化时，可能会在 JSON 中遇到未在结构体中声明的字段。将其一味的丢弃是会遭人唾弃的。
在 Accessor 的反序列化接口中，可以通过一个 `deserFlag` 类型的参数指定相关的行为：

- kNoneFlag： 默认选项，会保存 unknown 字段。
- kIgnoreUnknown： 忽略 unknown 字段。
- kErrorWhenUnknown： 遇到 unknown 字段时，返回错误。

如：

```cpp
// 反序列化时忽略 unknown 字段
std::string strJson = // Init...
int ret = sampleStruct._.Deserialize(strJson, ijst::DeserFlag::kIgnoreUnknown);
```

在序列化时，如未启用 `SerFlag::kIgnoreUnknown` 选项时，会输出的所有 unknown 字段。

## 访问 Unknown 字段

可以通过 Accessor 提供的 `GetUnknown()` 接口**访问和修改** Unknown 字段：

```cpp
rapidjson::Value& jUnknown = sampleStruct._.GetUnknown();
assert(jUnknown.IsObject() == true);
```

## Allocator

和 RapidJSON 一样，在修改原生的 `rapidjson::Value` 时，可能需要使用其配套的 allocator 对象。
ijst 提供了相关的接口获取和设置 allocator 对象：

```cpp
rapidjson::Value& jUnknown = sampleStruct._.GetUnknown();

// 获取当前使用的 allocator
rapidjson::MemoryPoolAllocator<>& alloc = sampleStruct._.GetAllocator();
jUnknown.SetString("s", 1, alloc);
```

默认情况下，每个结构体会使用单独的 allocator 复制 unknown 字段。这样会在最大程度上保证安全，但可能会带来额外的复制开销。
为此，反序列化接口提供了以下方法避免这个开销：

- `Deserialize()` 接口中，指定 `DeserFlag::kMoveFromIntermediateDoc` 选项。
- 使用 `MoveFromJson()` 接口。

这样的话，ijst 会使用父类的 allocator，并使用移动资源的方式避免 unknown 字段的复制。但这样会带来一些问题：

- 在对象析构前，源 doc （或反序列时产生的临时 doc）的 allocator 不会被释放。
- 若某嵌套对象使用右值拷贝的方式复制给另外一个对象，那么其仍然会使用源对象的 allocator，即父对象的 allocator。在源父对象析构后，其 allocator 会失效。

在使用这些方法时，需要更加小心。

另外，Accessor 也提供了 `ShrinkAllocator()` 方法，让所有嵌套的对象（包括自身）使用本身的 allocator 重新复制 unknown 字段，并释放原来的 allocator。

管理 allocator 是一个较为麻烦的事。ijst 中提供了 `GetOwnAllocator()` 接口以供有相关需求的使用者使用。


# Getter Chaining

JSON 提供了 JSON Pointer 以在不进行多次判断的情况下，快速地访问路径较深的字段。如使用 `/foo/bar/0` 可访问 `{"foo": {"bar": [0]}}` 中的成员。

ijst 也提供了类似的**静态类型**的方法：为每个字段定义 `get_` 方法，返回一个 `ijst::Optional` 对象，实现链式的 Getter：

```cpp
// 需使用 *_WITH_GETTER 宏
IJST_DEFINE_STRUCT_WITH_GETTER(
    StFoo
    , (IJST_TVEC(T_int), bar, "bar", 0)
)

IJST_DEFINE_STRUCT_WITH_GETTER(
    StOut
    , (IJST_TST(StFoo), foo, "foo", 0)
)

// 默认情况下会生成下面的定义
/*
class StFoo {
public:
    std::vector<int> bar;
    ijst::Optional<std::vector<int> > get_bar();
    // ...
}

class StOut {
public:
    StFoo foo;
    ijst::Optional<StFoo> get_foo();
    // ...
}
*/

StOut st;

// 直接访问 /foo/bar/0，而不用关心路径的中间节点是否存在
int* ptr = st.get_foo()->get_bar()[0].Ptr();
// 如果失败，最终结果是 nullptr
assert(ptr == NULL);
// 如果成功，ptr 会指向具体的字段，如 ptr == &st.foo.bar[0]
```

模板 `Optional<T>` 的实例中储存着一个指针，可以指向具体的值或 NULL。可以通过其 `Ptr()` 方法获得该指针。该模板针会对不同类型的进行特化：

- ijst 结构体类型。重载 `Optional<T> operator->()` 操作符，在指针为 NULL 时，返回一个 InValid 的结果。
- `std::vector<TElem>` 类型。重载 `Optional<TElem> operator[size_type i]` 操作符，在指针为 NULL，或 `i` 无效时，返回指向 NULL 的对象。
- `std::map<std::string, TElem>` 类型。重载 `Optional<TElem> operator[std::string& key]` 操作符，在指针为 NULL，或 `key` 无效时，返回指向 NULL 的对象。

另外，生成的 Getter 方法的默认前缀是 `get_`，可通过 `IJST_GETTER_PREFIX` 宏自定义该前缀。

注意，不同于 [C# 的 Null-conditional Operators](https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/operators/null-conditional-operators)，该操作**不**提供短路能力。

# 序列化/反序列化的其他选项

## 反序列化
在 `Deserialize()` 接口中，提供了一个模板参数 `parseFlags`，可以指定反序列化时的行为。
该参数会传递给 RapidJSON 的 `Parse()` 方法，可以在解析时忽略注释，尾部逗号等：

- 注释
```cpp
string json = "{/*This is a comment*/}";
st._.Deserialize<rapidjson::kParseCommentsFlag>(json.data(), json.size());
```

- 尾部的逗号
```cpp
string json = "{\"v\": 0, }";
st._.Deserialize<rapidjson::kParseTrailingCommasFlag>(json.c_str(), json.length());
```

更多的选项请见 [RapidJSON#Parsing](http://rapidjson.org/md_doc_dom.html#Parsing)。

## 序列化
在 `Serialize()` 接口中，可以传入 RapidJSON Handler，以实现特殊的需求。

如以下代码可使用 `rapidjson::PrettyWriter` 生成格式化的 JSON 字符串：
```cpp
rapidjson::StringBuffer buf;
rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
// 为便于实现，ijst 以继承的方式使用不同的 Handler，可以使用 ijst::HandlerWrapper 转换 RapidJSON Handler 模板
ijst::HandlerWrapper<rapidjson::PrettyWriter<rapidjson::StringBuffer> > writerWrapper(writer);
st->_.Serialize(writerWrapper);
```

另外，也可以通过 SAX 事件，直接生成 `rapidjson::Document` 对象：

```cpp
ijst::SAXGeneratorWrapper<rapidjson::Document> generator(st._, SerFlag::kNoneFlag);
rapidjson::Document doc;
doc.Populate(generator);
```


# Root as value

如果需要解析 root 为数组的 JSON，或是需要通过 `IJST_TMAP` 表达整个 JSON（即将 JSON 转成 map），则可以使用 `IJST_DEFINE_VALUE` 或 `IJST_DEFINE_VALUE_WITH_GETTER`。

- JSON 为数组:

```cpp
const std::string json = "[0, 1, 2]";

IJST_DEFINE_VALUE(
	VecVal, IJST_TVEC(T_int), v, 0
)

VecVal vec;
st._.Deserialize(json);
assert(st.v[2] == 2);
```

- 将 JSON 转成 map：

```cpp
const std::string json = R"(
{
    "v1": 2,
    "v2": 4,
    "v3": 8
})";

IJST_DEFINE_VALUE(
	MapVal, IJST_TMAP(T_int), v2, 0
)

MapVal vec;
st._.Deserialize(json);
assert(st.v2["v2"] == 4);
```

# UTF 编码

除默认情况下的 UTF-8 编码外，ijst 也支持 UTF-16，UTF-32 编码。这些编码的支持是建立在 RadpidJSON 的基础上的，所以在使用相关 API 时，需先了解 RapidJSON 的编码用法：[RapidJSON: Encoding][link]。

[link]: http://rapidjson.org/md_doc_encoding.html "RapidJson: Encoding"

## 序列化/反序列化时指定编吗

在序列化/反序列化时，可以通过以下方法指定输入/输出的编码：

```cpp
SampleStruct st;

// 反序列化时指定输入为 UTF-16 编码
const wchar_t* json = L"...";  // 假设平台的宽字符串为 UTF-16 鳊码
st._.Deserialize<rapidjson::kParseDefaultFlags, rapidjson::UTF16<> >(json);

// 序列化时指定编码
std::basic_string<wchar_t> out;
st._.Serialize<rapidjson::UTF16<> >(out);
```

## 定义结构体时指定编码

在上面例子中，会把 UTF-16 的输入，转换成 UTF-8 的 ijst 结构体，再在输出时转换为 UTF-16 的编码。

为避免这种转换，可以定义非 UTF-8 编码的 ijst 结构体（配合 C++ 11 使用更佳）：

```cpp
// 使用 IJST_DEFINE_GENERIC_STRUCT 宏，其他定义 ijst 结构体的宏也有相应的自定义编码的版本：
IJST_DEFINE_GENERIC_STRUCT (
	rapidjson::UTF16<char32_t>, U32SampleStruct       // 第一个参数指定编码
	, (ijst::T_int, iVal, u"int", 0)                  // 定义 JSON 键名时,使用 C++ 11 提供的 UTF-16 常量字符串（`u` 前缀）
	, (ijst::T_uint, uiVal, u"uint", 0)
	, (IJST_TSTR, strVal, u"str", 0)                  // 定义字符串时,使用 IJST_TSTR 宏。
	, (IJST_TRAW, rawVal, u"raw", 0)                  // 定义字符串时,使用 ISJT_TRAW 宏。
)
```

这样 ijst 就会在内部使用 UTF-16 编码进行处理。

注意到，这里引入了 `IJST_TSTR`，`IJST_TRAW` 两个宏，这是因为这两种类型需要依赖实际的编码。
而 `ijst::T_string`，`ijst::T_raw` 类型是 ijst 在未考虑多编码支持时设计出来的。为了兼容，保留了这两个类型。

# 其他

## 使用 extern template 加速编译

作为一个 header-only，且较多使用模板的库，编译性能是个值得注意的问题。
一般情况下，ijst 及其定义的结构体需要在每个 cpp 文件中都进行一次编译，这会带来额外的编译开销。

所幸 C++11 支持的 extern template，可以选择仅在一个 cpp 文件中实例化模板。 ijst 可以利用该特性，并根据宏定义确定其所用的模板的实例化行为，以加速编译：

```cpp
// header.h
#define IJST_EXTERN_TEMPLATE    // 默认阻止 ijst 所用模板的实例化
#include <ijst/ijst.h>


// f1.cpp, f2.cpp, f3.cpp, ...
#include "header.h"             // 在该文件中不会实例化模板


// explicit.cpp
#define IJST_EXPLICIT_TEMPLATE  // 在该文件中显式实例化模板
#include "header.h"
```

## 元信息

ijst 在实现时需要记录相关的元信息，也提供了接口获取这些信息：

```cpp
// 直接获取某个结构的元信息
const ijst::MetaClassInfo& metaInfo = ijst::MetaClassInfo::GetMetaInfo<SampleStruct>();
// 或通过 Accessor 获取
SampleStruct st;
metaInfo = st._.GetMetaInfo();

// 通过 metaInfo 可以访问元信息
```

ijst 记录的元信息包括字段的偏移量、名字、对应的 JSON 键名、FieldDesc 等。
注意因为 C++ 较难在运行时使用类型信息，所以元信息中并没有记录。

## 错误信息

ijst 提供 JSON 格式的错误信息，以在反序列化复杂的结构出错时快速定位：

```cpp
// 声明的 id 为 int，但实际为 string
const string json = "{\"id\": \"ThisIsAString\"}";
SampleStruct st;
string errMsg;
st._.Deserialize(json, errMsg);
// errMsg = {"type":"ErrInObject","member":"iId","jsonKey":"id","err":{"type":"TypeMismatch","expectedType":"int","json":"\"ThisIsAString\""}}
```