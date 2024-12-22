#pragma once

#include "../utilities/debug.hpp"
#include "../../resources/memcury.h"
#include "../../resources/minhook/minhook.h"

#include <windows.h>

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;

class FMemory
{
public:
	static inline void (*FreeInternal)(void*);
	static inline void (*ReallocInternal)(void*);
};

template <typename T>
class TArray
{
	friend struct FString;
public:
	T* Data;
	int32 ArrayNum, ArrayMax = 0;

	int32 Capacity()
	{
		return ArrayMax;
	}

	int32 Num()
	{
		return ArrayNum;
	}

	inline T* begin()
	{
		return(T*)(ArrayNum, Data);
	}

	inline T* end()
	{
		return(T*)(ArrayNum, Data + ArrayNum);
	}
};

class FString : private TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		ArrayMax = ArrayNum = *other ? std::wcslen(other) + 1 : 0;

		if (ArrayNum)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	auto GetData()
	{
		return Data;
	}

	std::string ToString()
	{
		auto length = std::wcslen(Data);

		std::string str(length, '\0');

		std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

		return str;
	}
};

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;

	static inline void (*ToStringInternal)(FName*, FString&);

	std::string ToString()
	{
		FString Buffer;
		ToStringInternal(this, Buffer);

		auto Return = std::string(Buffer.ToString());
		FMemory::FreeInternal((void*)Buffer.GetData());
		return Return;
	}
};

class UObject
{
public:
	void** VTable;
	int32 ObjectFlags;
	int32 IntenralIndex;
	UObject* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;

	static inline void* (*ProcessEventInternal)(UObject*, class UFunction*, void*);
	inline void* ProcessEvent(class UFunction*, void*);

	static inline UObject* (*StaticFindObjectInternal)(UObject*, void*, const wchar_t*, bool);
	static inline UObject* (*StaticLoadObjectInternal)(UObject*, UObject*, const wchar_t*, const wchar_t*, uint32, void*, bool, void*);

	std::string GetName()
	{
		return NamePrivate.ToString();
	}

	std::string GetFullName()
	{
		std::string temp;

		for (auto outer = this->OuterPrivate; outer; outer = outer->OuterPrivate)
		{
			temp = outer->GetName() + "." + temp;
		}

		temp = reinterpret_cast<UObject*>(ClassPrivate)->GetName() + " " + temp + this->GetName();

		return temp;
	}

	template <typename T = UObject*>
	T& Property(const std::string& Name);

	template< typename T = void*, int16_t FallbackReturnValueOffset = -1, typename ...Parameters >
	T Function(const std::string& Name, Parameters... Arguments);

	template <typename T = UObject>
	static inline T* Object(std::string Name, bool bFallback = false,
		UObject* Class = nullptr, UObject* InOuter = nullptr)
	{
		auto Wide = std::wstring(Name.begin(), Name.end());

		auto Return = StaticFindObjectInternal(nullptr, nullptr, Wide.c_str(), false);

		if (!Return && bFallback)
			Return = StaticLoadObjectInternal(nullptr, nullptr, Wide.c_str(), nullptr, 0, nullptr, false, nullptr);

		return (T*)Return;
	}
};

struct UField : UObject
{
	UField* Next()
	{
		return *reinterpret_cast<UField**>(int64(this) + 0x28);
	}
};

class UProperty : public UField
{
public:
	int32 OffsetInternal()
	{
		return *reinterpret_cast<int32*>(reinterpret_cast<uintptr_t>(this) + 0x44);
	}
};

struct UStruct : public UField
{
	UStruct* SuperStruct()
	{
		return *reinterpret_cast<UStruct**>(reinterpret_cast<uintptr_t>(this) + 0x30);
	}

	UField* ChildProperties()
	{
		return *reinterpret_cast<UField**>(reinterpret_cast<uintptr_t>(this) + 0x38);
	}
};

class UFunction : public UStruct
{
public:
	uint16 ParmsSize()
	{
		return *(int16_t*)(int64_t(this) + 0x8E);
	}

	uint16 ReturnValueOffset()
	{
		return *(int16_t*)(int64_t(this) + 0x90);
	}
	
	void*& Func()
	{
		return *reinterpret_cast<void**>(reinterpret_cast<int64>(this) + 0xB0);
	}

	std::vector<int32> ChildrenOffsets()
	{
		auto Return = std::vector<int32>();
		auto Children = this->ChildProperties();

		while (true)
		{
			if (!Children)
				break;

			auto ToChildren = (UProperty*)Children;
			Return.push_back(ToChildren->OffsetInternal());
			Children = Children->Next();
		}

		return Return;
	}
};

void* UObject::ProcessEvent(UFunction* Function, void* Parameters = nullptr)
{
	return ProcessEventInternal(this, Function, Parameters);
}

template <typename T>
T& UObject::Property(const std::string& Name)
{
	UObject* Child = nullptr;
	auto Class = reinterpret_cast<UStruct*>(this->ClassPrivate);

	while (Class)
	{
		auto ChildProperties = Class->ChildProperties();

		while (ChildProperties)
		{
			if (ChildProperties->GetName() == Name)
			{
				Child = reinterpret_cast<UObject*>(ChildProperties);
				break;
			}

			ChildProperties = ChildProperties->Next();
		}

		if (Child)
		{
			break;
		}

		Class = Class->SuperStruct();
	}

	return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(this) + ((UProperty*)Child)->OffsetInternal());
}

template <typename T, int16_t FallbackReturnValueOffset, typename... Parameters>
T UObject::Function(const std::string& Name, Parameters... args)
{
	UFunction* Func = nullptr;
	auto Class = reinterpret_cast<UStruct*>(this->ClassPrivate);

	while (Class)
	{
		auto Children = Class->ChildProperties();

		while (Children)
		{
			if (Children->GetName() == Name)
			{
				Func = reinterpret_cast<UFunction*>(Children);
				break;
			}
			Children = Children->Next();
		}

		if (Func) break;
		Class = Class->SuperStruct();
	}

	auto ParmsSize = Func->ParmsSize();
	auto ReturnValueOffset = Func->ReturnValueOffset();

	auto ToParameters = std::make_unique<uint8_t[]>(ParmsSize);

	int i = 0;
	auto Offsets = Func->ChildrenOffsets();

	std::apply([&](auto... argument) {
		((memcpy(ToParameters.get() + Offsets[i++], argument, sizeof(*argument))), ...);
		}, std::make_tuple(&args...));

	ProcessEventInternal(this, Func, ToParameters.get());

	if (ReturnValueOffset == static_cast<uint16>(-1))
		ReturnValueOffset = FallbackReturnValueOffset;

	return *reinterpret_cast<T*>(ToParameters.get() + ReturnValueOffset);
}

struct FVector
{
	float X;
	float Y;
	float Z;

	bool operator==(const FVector& Vector)
	{
		return X == Vector.X && Y == Vector.Y && Z == Vector.Z;
	}

	auto operator-(FVector A)
	{
		return FVector{ this->X - A.X, this->Y - A.Y, this->Z - A.Z };
	}

	auto operator+(FVector A)
	{
		return FVector{ this->X + A.X, this->Y + A.Y, this->Z + A.Z };
	}

	auto operator!=(FVector A) const
	{
		return (this->X != A.X && this->Y != A.Y && this->Z != A.Z);
	}

	auto operator|(const FVector& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	operator bool() const
	{
		return X != 0 && Y != 0 && Z != 0;
	}
};

struct alignas(16) FQuat
{
	float X;
	float Y;
	float Z;
	float W;
};

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;
};

struct alignas(16) FTransform
{
	struct FQuat Rotation;
	struct FVector Translation;
	unsigned char UnknownData00[0x4];
	struct FVector Scale3D;
	unsigned char UnknownData01[0x4];
};