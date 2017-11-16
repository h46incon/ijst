## ijst

ijst (iJsonStruct) 是C++ Json序列化库：

- 使用方便。只需在C++中定义一次结构体，无须重复添加元信息，更不用额外工具生成代码。
- 完整。支持list、map以及结构体的嵌套定义。
- 轻量。header-only，仅依赖stl和[rapidjson](https://github.com/Tencent/rapidjson)
- 兼容C++03。

## 用法
### 安装
1. 将 rapidjson 加入头文件搜索路径
2. 将`include/ijst`文件夹复制进工程

注：ijst需要依赖我给rapidjson提交的一小小段代码，但目前的release版本（v1.1.0）还未包含该提交，所以请通过git签出其master分支。

### 基本使用
```cpp
#include <ijst/ijst.h>
#include <ijst/types_std.h>

// json字符串
const std::string jsonStr = R"({
"int_val": 42, 
"vec_val": ["str1", "str2"], 
"map_val": {"k1": 1, "k2": 2}
})";

// 使用ijst唯一的额外工作就是定义一个ijst结构体：
IJST_DEFINE_STRUCT(
    // 结构体名字
    JsonStruct
    // 定义字段
    , (IJST_TPRI(Int), iVal, "int_val", 0)  
    , (IJST_TVEC(IJST_TPRI(Str)), vecVal, "vec_val", 0)
    , (IJST_TMAP(IJST_TPRI(UInt64)), mapVal, "map_val", 0)
);


// 定义一个JsonStruct对象
JsonStruct jStruct;

// 反序列化
int ret = jStruct._.Deserialize(jsonStr);
assert(ret == 0);

// 访问字段
assert(jStruct.iVal == 42);
assert(jStruct.vecVal[0] == "str1");
assert(jStruct.mapVal["k2"] == 2);

// 序列化
std::string strOut;
ret = jStruct._.SerializeToString(/*pushAllField=*/true, strOut);
assert (ret == 0);
```