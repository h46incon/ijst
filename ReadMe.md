## ijst

ijst (iJsonStruct) 一个是 C++ Json 序列化/反序列化库：

- 只需定义**一次**结构体，无须重复添加元信息。
- 支持 Getter Chaining，可以很简单地访问路径较深的字段。
- 支持 unknown 字段和可选字段。
- 轻量。header-only，仅依赖 stl 和 [rapidJSON](https://github.com/Tencent/rapidjson)。
- 兼容 C++98/03。支持 C++11 特性，如右值构造、extern template 等。

## 使用
### 安装
1. 安装 rapidJSON v1.1.0 以上版本（将其加入 header 搜索路径即可）。
2. 将 `include/ijst` 文件夹复制进工程。

### 基本使用

#### 定义结构体
```cpp
#include <ijst/ijst.h>
#include <ijst/types_std.h>
#include <ijst/types_container.h>
using namespace ijst;

//*** 需要反序列化的 JSON 字符串
const std::string jsonStr = R"(
{
    "int_val": 42, 
    "vec_val": ["str1", "str2"], 
    "map_val": {"k1": 1, "k2": 2}
})";

//*** 定义一个 ijst 结构体：
IJST_DEFINE_STRUCT(
    // 结构体名字
    JsonStruct
    // 定义字段
    , (T_int, iVal, "int_val", 0)  
    , (IJST_TVEC(T_string), vecVal, "vec_val", 0)
    , (IJST_TMAP(T_uint64), mapVal, "map_val", 0)
);

//*** 默认情况下会生成这样的类：
/*
class JsonStruct {
public:
    ijst::Accessor _;   // 通过这个对象进行序列化等操作
    int iVal; 
    std::vector<std::string> vecVal; 
    std::map<std::string, uint64_t> mapVal; 

private:
    //... Some private methods
};
*/
```

#### 字段访问及(反)序列化
```cpp
//*** 定义一个 JsonStruct 对象
JsonStruct jStruct;

//*** 反序列化
int ret = jStruct._.Deserialize(jsonStr);
assert(ret == 0);

//*** 访问字段
assert(jStruct.iVal == 42);
assert(jStruct.vecVal[0] == "str1");
assert(jStruct.mapVal["k2"] == 2);

//*** 序列化
std::string strOut;
ret = jStruct._.Serialize(strOut);
assert (ret == 0);
```

### Getter Chaining
如果所需访问的字段的路径比较深的时候，为避免累赘的判断，可使用 `get_*` 方法，比如：

```cpp
int* ptr = st.get_vecField()[0].get_mapField()["key"].get_intField().Ptr();
if (ptr != NULL)  //...
```

具体如下：

```cpp
//*** 和 IJST_DEFINE_STRUCT 类似
IJST_DEFINE_STRUCT_WITH_GETTER(
    StIn
    , (T_int, iData, "i", ijst::FDesc::Optional)
    , (IJST_TVEC(T_int), vecData, "vec", ijst::FDesc::Optional)
    , (IJST_TMAP(T_int), mapData, "map", ijst::FDesc::Optional)
)

//*** 默认情况下会生成这样的结构体：
/*
class JsonStruct {
public:
    //... 普通的字段，同 IJST_DEFINE_STRUCT
    
    // Getters
    ijst::Optional<T_int> get_iData();
    ijst::Optional<std::vector<T_int> > get_vecData();
    ijst::Optional<std::map<std::string, T_int> > get_mapData();
private:
};
*/

IJST_DEFINE_STRUCT_WITH_GETTER(
    StOut
    , (IJST_TST(StOut), stIn, "inner", ijst::FDesc::Optional)
)

//*** 可以通过连串的 get_* 尝试直接访问字段，而不用关注路径的中间节点是否存在
StOut st;
int* pData = st.get_stIn()->get_vecData()[2].Ptr();
assert (pData == NULL);
// 如果路径中的每个字段都是 kValid 的，且 vector 或 map 中的下标或键存在，则最终得到的指针会指向该字段：
// int* pData = st.get_stIn()->get_vecData()[2].Ptr() == &st.stIn.vecData[2];
```

### 性能

ijst 底层使用的是 rapidJSON，其本身具有优秀的性能。
ijst 因有额外的工作，会带来一些性能上的开销，但也比常用的 JsonCpp 快上不少：

| Library   | 序列化 | 反序列化 | 
|-----------|-------|---------|
| rapidJSON | 14    | 10      |
| ijst      | 16    | 23      |
| JsonCpp   | 128   | 109     |

测试环境：Corei7-4790@3.60GHz_vc2017_win64，测试代码[nativejson-benchmark](https://github.com/miloyip/nativejson-benchmark)。
 
注：不同环境测得的性能会有差异，一般而言，ijst 的序列化性能和 RapidJSON 相似，反序列化性能为其 1/4 ~ 1/2。

### 详细说明

ijst 的其他功能，如字段状态，Unknown 字段，JSON 注释等，请移步 [Usage](docs/usage.md)，或 `docs/Doxygen/html`。