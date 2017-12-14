## 定义结构体

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

通过该宏可以指定结构体的名字和每个字段的信息。该宏会声明相关的 class，以及注册相关的元信息。

一个例子：

```cpp
#include <ijst/ijst.h>
#include <ijst/types_std.h>

IJST_DEFINE_STRUCT (
    SampleStruct
    , (IJST_TPRI(Int), iID, "id", 0)
    , (IJST_TPRI(Str), strName, "name", 0)
    , (IJST_TPRI(Bool), bSex, "sex", 0)     // 只是举一个bool的栗子
    // 接下来是复杂的字段
    , (IJST_TVEC(IJST_TPRI(UInt64)), vecFriendsID, "friends_id", ijst::FDesc::Optional) // Optional，可能没朋友
    , (IJST_TMAP(IJST_TPRI(Str)), mapWhatEver, "what_ever", ijst::FDesc::ElemNotEmpty)
);

// 定义该类型的变量
SampleStruct sampleStruct;
```

宏中每个参数的定义如下：

### StructName

结构体名字，没有特殊的限制。

### FieldType

字段类型。该类型是预定义的类型，需要通过 ijst 提供的宏指定。

目前已实现的类型如下：

\*\* **基本原子类型**

在 `ijst/types_std.h` 中定义。提供的宏为 `IJST_TPRI(_type)`。

ijst 提供了一系列的原子类型，包括 `Int, Bool, Int, UInt32, UInt64, Int32, Int64, Str, Raw`。
如 `IJST_TPRI(Int)` 即可声明一个 int 型字段。这些原子类型可以满足大部分的使用需求。

如果不能确定某个字段的类型，则可以使用 `Raw` 类型操作原始的 `rapidjson::Value` 对象。

\*\* **时间原子类型**

在 `ijst/types_time.h` 中定义。提供的宏为 `IJST_TTIME(), IJST_TFTIME(_time_zone)`。

这两个类型会将格式为 *YYYY-MM-DD HH:MM:SS* 的字符串转换为 unix 时间戳。
其中 `IJST_TTIME()` 会调用系统调用确定时区（**挺慢的**），`IJST_TFTIME(_time_zone)` 可以使用参数指定的时区，如 `IJST_TFTIME(8)` 指定时区为东八区。

\*\* **容器类型**

在 `ijst/ijst.h` 中定义。提供的宏为 `IJST_TVEC(_type), IJST_TMAP(_type)`。

ijst 分别使用这两个宏表达 json 中的 list 和任意键值的 object，和 json 中一样，容器的元素类型可以为原子类型和容器（即支持**嵌套**定义）。
如 `IJST_TVEC(IJST_TRPI(Int))` 可表达 json 值 *[1, 2, 3]*， `IJST_TMAP(IJST_TVEC(IJST_TPRI(Bool)))` 可表达 json 值 *{"key1": [true, true], "key2": [true, false]}*。

\*\* **ijst 结构体类型**

在 `ijst/ijst.h` 中定义。提供的宏为 `IJST_TOBJ(_type)`。

可使用该宏在结构体中包含其他的结构体，如 `IJST_TOBJ(struct_name)`。

\*\* **自定义类型**

如有特殊需求，可参考代码自行添加类型。仅需实现相应的序列化\反序列接口(`ijst::detail::SerializeInterface`)即可。

### FieldName

成员变量名。

- 如果使用 `IJST_DEFINE_STRUCT`，变量名不能为 `_`（一个下划线）
- 如果使用 `IJST_DEFINE_STRUCT_WITH_GETTER`，变量名不能为 `_`， `_TypeDef`， `Get变量名`。

### JsonName

json 中的健值。

### FieldDesc

字段描述。该值可以为`ijst::FDesc`中值的组合，如无特殊情况，使用0即可。值的含义如下：

- Optional：该字段可能在 json 中不出现。
- Nullable：该字段的 json 值可能为 null。
- ElemNotEmpty：该字段是个容器类型，且至少有一个元素。

---

## 接口

在定义了一个 ijst 结构体后，除了用户定义的字段，其还会添加一个名为 `_` 的 `Accessor` 类型成员。
通过该成员可以完成对象的序列化、反序列化等操作。

```cpp
int ret;

// 序列化
std::string strOut;
ret = sampleStruct._.SerializeToString(/*pushAllField=*/true, strOut);

// 反序列化
std::string strJson = // Init...
ret = sampleStruct._.Deserialize(strJson);

// 访问 Unknown 字段
rapidjson::Value& jUnknown = sampleStruct._.GetBuffer();
```

另外，在某些情况下，只需关心序列化/反序列后的结果，而不需关心其源对象是否会被破坏，则可以用 **Move** 类型的接口以提高效率。这些接口会尝试使用窃取资源的方式以减少拷贝（thanks rapidjson）：

```cpp
int ret;

// Move 序列化，序列化后 sampleStruct 对象的 Unknown 会被窃取
std::string strOut;
ret = sampleStruct._.MoveSerializeToString(/*pushAllField=*/true, strOut);

// Move 反序列化，反序列化后 doc 对象的内容会被窃取
rapidjson::Document doc;
ret = sampleStruct._.MoveDeserialize(doc);

// Parse insitu 的方式，从字符串中反序列化。
// 反序列化后源字符串会被修改。需注意其生命周期
char* strJson = new char[N]; //Init...
ret = sampleStruct._.DeserializeInsitu(strJson);
// ... after ~sampleStruct()
delete[] strJson;

```

这些例子中省略了一些默认参数。可通过这些参数指定序列化/反序列化时的具体行为。
完整的接口定义请参考 API 文档（如果我整理了），或直接阅读源码中的函数说明。
但是在阅读 API 文档前，建议继续往下阅读以得到大致的了解。

---

## 字段的状态

ijst 会记录每个字段的状态，这些状态会影响**序列化**时的行为。可能的状态如下：

- kMissing：未设置有效值。如指定 `pushAllField=false`，则不参与序列化。
- kParseFailed：解析该字段时出错。如指定 `pushAllField=false`，则不参与序列化。
- kNull：值为 null。序列化时值为 null。
- kValid：已设置为有效值。按实际值序列化。

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
// 或使用 SET 宏在设置字段值的同时，将其状态设置为 valid
IJST_SET(st, strName, "unique name");
assert( IJST_GET_STATUS(st, strName) == ijst::FStatus::kValid );
```

---

## Unknown

### 反序列化时的行为

在反序列化时，可能会在 json 中遇到未在结构体中声明的字段。将其一味的丢弃是会遭人唾弃的。
在 ijst 的反序列化接口中，可以通过一个 `ijst::EUnknownMode` 类型的参数指定相关的行为：

- kIgnore：忽略 unknown 字段。
- kKeep： 保存 unknown 字段。
- kError： 遇到 unknown 字段时，返回错误。

如：

```cpp
// 反序列化时忽略 unknown 字段
std::string strJson = // Init...
int ret = sampleStruct._.Deserialize(strJson, ijst::UnknownMode::kIgnore);
```

在序列化时，会输出对象保存的所有 unknown 字段。

### 访问 Unknown 字段

可以通过 ijst 提供的 `GetBuffer()` 接口**访问和修改** Unknown 字段：

```cpp
rapidjson::Value& jUnknown = sampleStruct._.GetBuffer();
assert(jUnknown.IsObject() == true);
```

### Allocator

和 rapidjson 一样，在修改原生的 `rapidjson::Value` 时，可能需要使用其配套的 allocator 对象。
ijst 提供了相关的接口获取和设置 Allocator 对象：

```cpp
ijst::JsonValue& jUnknown = sampleStruct._.GetBuffer();

// 获取当前使用的 allocator 
ijst::JsonAllocator& alloc = sampleStruct._.GetAllocator();
jUnknown.SetString("s", 1, alloc);
```

如果定义了嵌套了的其他结构体的结构体，且需要修改其 unknown 字段。
可以先将他们的 allocator 设置为相同的值，这样在 Move 序列化的时候，可以减少拷贝：

```cpp
IJST_DEFINE_STRUCT (
    OuterStruct
    , (IJST_TOBJ(SampleStruct), stSample, "sample", 0)
);
OuterStruct ost;

ost._.InitMembersAllocator();
// 或
ost._.SetMembersAllocator(otherAlloc);
assert(&ost._.GetAllocator() == &ost.stSample._.GetAllocator());
```

注：在反序列化的过程中，已经自动执行了相同效果的动作。

也和 rapidjson 一样，管理 allocator 是一个较为麻烦的事。ijst 中提供了 `GetOwnAllocator()` 接口以供有相关需求的使用者使用。
