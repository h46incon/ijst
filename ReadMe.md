## ijst

ijst (iJsonStruct) 一个是 C++ Json 序列化库：

- 使用方便。只需在 C++ 中定义一次结构体，无须重复添加元信息，更不用额外工具生成代码。
- 完整。支持 list、map 以及结构体的嵌套定义。
- 轻量。header-only，仅依赖 stl 和 [rapidjson](https://github.com/Tencent/rapidjson)。
- 兼容 C++ 98/03。

## 使用
### 安装
1. 将 rapidjson 加入头文件搜索路径
2. 将 `include/ijst` 文件夹复制进工程

注：ijst 需要依赖一些 rapidjson 中未 release 的提交，所以请通过 git 签出其 master 分支。

### 基本使用

#### 定义结构体
```cpp
#include <ijst/ijst.h>
#include <ijst/types_std.h>

//*** json 字符串
const std::string jsonStr = R"({
"int_val": 42, 
"vec_val": ["str1", "str2"], 
"map_val": {"k1": 1, "k2": 2}
})";

//*** 使用 ijst 唯一的额外工作就是定义一个 ijst 结构体：
IJST_DEFINE_STRUCT(
    // 结构体名字
    JsonStruct
    // 定义字段
    , (IJST_TPRI(Int), iVal, "int_val", 0)  
    , (IJST_TVEC(IJST_TPRI(Str)), vecVal, "vec_val", 0)
    , (IJST_TMAP(IJST_TPRI(UInt64)), mapVal, "map_val", 0)
);

//*** 默认情况下会生成这样的结构体：
/*
class JsonStruct {
public:
    ijst::Accessor _;   // API 入口
    int iVal; 
    std::vector<std::string> vecVal; 
    std::map<std::string, uint64_t> mapVal; 

private:
    //... Some private methods
};
*/
```

#### 字段访问及序列化
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
ret = jStruct._.Serialize(/*pushAllField=*/true, strOut);
assert (ret == 0);
```

### Getter Chaining
如果所需字段的路径比较深的时候，为避免累赘的判断，可使用 `Get*` 方法，比如：

```cpp
int* ptr = st.GetvecField()[0].GetmapField()["key"].GetintField().Ptr();
if (ptr != NULL)  //...
```

具体如下：

```cpp
//*** 和 IJST_DEFINE_STRUCT 类似
IJST_DEFINE_STRUCT_WITH_GETTER(
    Inner
    , (IJST_TPRI(Int), iData, "i", ijst::FDesc::Optional)
    , (IJST_TVEC(IJST_TPRI(Int)), vecData, "vec", ijst::FDesc::Optional)
    , (IJST_TMAP(IJST_TPRI(Int)), mapData, "map", ijst::FDesc::Optional)
)

//*** 默认情况下会生成这样的结构体：
/*
class JsonStruct {
public:
    //... 普通的字段，同 IJST_DEFINE_STRUCT
    struct _TypeDef {
        typedef int iData;
        typedef std::vector<int> vecData;
        typedef std::map<std::string, int> mapData;
    };
    // Getters
    ijst::Optional<_TypeDef::iData> GetiData();
    ijst::Optional<_TypeDef::vecData> GetvecData();
    ijst::Optional<_TypeDef::mapData> GetmapData();
private:
};
*/

IJST_DEFINE_STRUCT_WITH_GETTER(
    Outter
    , (IJST_TOBJ(Inner), stInner, "inner", ijst::FDesc::Optional)
)


//*** 可以通过连串的 Get 尝试直接访问字段
Outter stOutter;
int* pData = stOutter.GetstInner()->GetvecData()[2].Ptr();
if (pData != NULL) {
    // ... 
}
```

### 详细说明
移步 [Usage](doc/Usage.md)。