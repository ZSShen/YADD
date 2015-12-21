
#ifndef _ART_DEX_FILE_H_
#define _ART_DEX_FILE_H_


#include "globals.h"
#include "macros.h"
#include "log.h"
#include "scoped_fd.h"
#include "scoped_map.h"

#include "modifiers.h"
#include "invoke_type.h"
#include "leb128.h"
#include "stringpiece.h"


class Signature;

class DexFile
{
  public:
    static const byte kDexMagic[];
    static const byte kDexMagicVersion[];
    static const size_t kSha1DigestSize = 20;
    static const uint32_t kDexEndianConstant = 0x12345678;

    // name of the DexFile entry within a zip archive.
    static const char* kClassesDex;

    // The value of an invalid index.
    static const uint32_t kDexNoIndex = 0xFFFFFFFF;

    // The value of an invalid index.
    static const uint16_t kDexNoIndex16 = 0xFFFF;

    // The separator charactor in MultiDex locations.
    static constexpr char kMultiDexSeparator = ':';

    // A string version of the previous. This is a define so that we can merge string literals in the
    // preprocessor.
    #define kMultiDexSeparatorString ":"

    // Raw header_item.
    struct Header
    {
        uint8_t magic_[8];
        uint32_t checksum_;  // See also location_checksum_
        uint8_t signature_[kSha1DigestSize];
        uint32_t file_size_;  // size of entire file
        uint32_t header_size_;  // offset to start of next section
        uint32_t endian_tag_;
        uint32_t link_size_;  // unused
        uint32_t link_off_;  // unused
        uint32_t map_off_;  // unused
        uint32_t string_ids_size_;  // number of StringIds
        uint32_t string_ids_off_;  // file offset of StringIds array
        uint32_t type_ids_size_;  // number of TypeIds, we don't support more than 65535
        uint32_t type_ids_off_;  // file offset of TypeIds array
        uint32_t proto_ids_size_;  // number of ProtoIds, we don't support more than 65535
        uint32_t proto_ids_off_;  // file offset of ProtoIds array
        uint32_t field_ids_size_;  // number of FieldIds
        uint32_t field_ids_off_;  // file offset of FieldIds array
        uint32_t method_ids_size_;  // number of MethodIds
        uint32_t method_ids_off_;  // file offset of MethodIds array
        uint32_t class_defs_size_;  // number of ClassDefs
        uint32_t class_defs_off_;  // file offset of ClassDef array
        uint32_t data_size_;  // unused
        uint32_t data_off_;  // unused

      private:
        DISALLOW_COPY_AND_ASSIGN(Header);
    };

    // Map item type codes.
    enum
    {
        kDexTypeHeaderItem               = 0x0000,
        kDexTypeStringIdItem             = 0x0001,
        kDexTypeTypeIdItem               = 0x0002,
        kDexTypeProtoIdItem              = 0x0003,
        kDexTypeFieldIdItem              = 0x0004,
        kDexTypeMethodIdItem             = 0x0005,
        kDexTypeClassDefItem             = 0x0006,
        kDexTypeMapList                  = 0x1000,
        kDexTypeTypeList                 = 0x1001,
        kDexTypeAnnotationSetRefList     = 0x1002,
        kDexTypeAnnotationSetItem        = 0x1003,
        kDexTypeClassDataItem            = 0x2000,
        kDexTypeCodeItem                 = 0x2001,
        kDexTypeStringDataItem           = 0x2002,
        kDexTypeDebugInfoItem            = 0x2003,
        kDexTypeAnnotationItem           = 0x2004,
        kDexTypeEncodedArrayItem         = 0x2005,
        kDexTypeAnnotationsDirectoryItem = 0x2006,
    };

    struct MapItem
    {
        uint16_t type_;
        uint16_t unused_;
        uint32_t size_;
        uint32_t offset_;

      private:
        DISALLOW_COPY_AND_ASSIGN(MapItem);
    };

    struct MapList
    {
        uint32_t size_;
        MapItem list_[1];

      private:
        DISALLOW_COPY_AND_ASSIGN(MapList);
    };

    // Raw string_id_item.
    struct StringId
    {
        uint32_t string_data_off_;  // offset in bytes from the base address

      private:
        DISALLOW_COPY_AND_ASSIGN(StringId);
    };

    // Raw type_id_item.
    struct TypeId
    {
        uint32_t descriptor_idx_;  // index into string_ids

      private:
        DISALLOW_COPY_AND_ASSIGN(TypeId);
    };

    // Raw field_id_item.
    struct FieldId
    {
        uint16_t class_idx_;  // index into type_ids_ array for defining class
        uint16_t type_idx_;  // index into type_ids_ array for field type
        uint32_t name_idx_;  // index into string_ids_ array for field name

      private:
        DISALLOW_COPY_AND_ASSIGN(FieldId);
    };

    // Raw method_id_item.
    struct MethodId
    {
        uint16_t class_idx_;  // index into type_ids_ array for defining class
        uint16_t proto_idx_;  // index into proto_ids_ array for method prototype
        uint32_t name_idx_;  // index into string_ids_ array for method name

      private:
        DISALLOW_COPY_AND_ASSIGN(MethodId);
    };

    // Raw proto_id_item.
    struct ProtoId
    {
        uint32_t shorty_idx_;  // index into string_ids array for shorty descriptor
        uint16_t return_type_idx_;  // index into type_ids array for return type
        uint16_t pad_;             // padding = 0
        uint32_t parameters_off_;  // file offset to type_list for parameter types

      private:
        DISALLOW_COPY_AND_ASSIGN(ProtoId);
    };

    // Raw class_def_item.
    struct ClassDef
    {
        uint16_t class_idx_;  // index into type_ids_ array for this class
        uint16_t pad1_;  // padding = 0
        uint32_t access_flags_;
        uint16_t superclass_idx_;  // index into type_ids_ array for superclass
        uint16_t pad2_;  // padding = 0
        uint32_t interfaces_off_;  // file offset to TypeList
        uint32_t source_file_idx_;  // index into string_ids_ for source file name
        uint32_t annotations_off_;  // file offset to annotations_directory_item
        uint32_t class_data_off_;  // file offset to class_data_item
        uint32_t static_values_off_;  // file offset to EncodedArray

        // Returns the valid access flags, that is, Java modifier bits relevant to the ClassDef type
        // (class or interface). These are all in the lower 16b and do not contain runtime flags.
        uint32_t GetJavaAccessFlags() const
        {
            if ((access_flags_ & kAccInterface) != 0) {
                // Interface.
                return access_flags_ & kAccValidInterfaceFlags;
            } else {
                // Class.
                return access_flags_ & kAccValidClassFlags;
            }
        }

      private:
        DISALLOW_COPY_AND_ASSIGN(ClassDef);
    };

    // Raw type_item.
    struct TypeItem
    {
        uint16_t type_idx_;  // index into type_ids section

      private:
        DISALLOW_COPY_AND_ASSIGN(TypeItem);
    };

    // Raw type_list.
    class TypeList
    {
      public:
        uint32_t Size() const
        {
            return size_;
        }

        const TypeItem& GetTypeItem(uint32_t idx) const
        {
            CHECK_LT(idx, this->size_);
            return this->list_[idx];
        }

        // Size in bytes of the part of the list that is common.
        static constexpr size_t GetHeaderSize()
        {
            return 4U;
        }

        // Size in bytes of the whole type list including all the stored elements.
        static constexpr size_t GetListSize(size_t count)
        {
            return GetHeaderSize() + sizeof(TypeItem) * count;
        }

      private:
        uint32_t size_;  // size of the list, in entries
        TypeItem list_[1];  // elements of the list
        DISALLOW_COPY_AND_ASSIGN(TypeList);
    };

    // Raw code_item.
    struct CodeItem
    {
        uint16_t registers_size_;
        uint16_t ins_size_;
        uint16_t outs_size_;
        uint16_t tries_size_;
        uint32_t debug_info_off_;  // file offset to debug info stream
        uint32_t insns_size_in_code_units_;  // size of the insns array, in 2 byte code units
        uint16_t insns_[1];

      private:
        DISALLOW_COPY_AND_ASSIGN(CodeItem);
    };

    // Raw try_item.
    struct TryItem
    {
        uint32_t start_addr_;
        uint16_t insn_count_;
        uint16_t handler_off_;

      private:
        DISALLOW_COPY_AND_ASSIGN(TryItem);
    };

    // Annotation constants.
    enum
    {
        kDexVisibilityBuild         = 0x00,     /* annotation visibility */
        kDexVisibilityRuntime       = 0x01,
        kDexVisibilitySystem        = 0x02,

        kDexAnnotationByte          = 0x00,
        kDexAnnotationShort         = 0x02,
        kDexAnnotationChar          = 0x03,
        kDexAnnotationInt           = 0x04,
        kDexAnnotationLong          = 0x06,
        kDexAnnotationFloat         = 0x10,
        kDexAnnotationDouble        = 0x11,
        kDexAnnotationString        = 0x17,
        kDexAnnotationType          = 0x18,
        kDexAnnotationField         = 0x19,
        kDexAnnotationMethod        = 0x1a,
        kDexAnnotationEnum          = 0x1b,
        kDexAnnotationArray         = 0x1c,
        kDexAnnotationAnnotation    = 0x1d,
        kDexAnnotationNull          = 0x1e,
        kDexAnnotationBoolean       = 0x1f,

        kDexAnnotationValueTypeMask = 0x1f,     /* low 5 bits */
        kDexAnnotationValueArgShift = 5,
    };

    struct AnnotationsDirectoryItem
    {
        uint32_t class_annotations_off_;
        uint32_t fields_size_;
        uint32_t methods_size_;
        uint32_t parameters_size_;

      private:
        DISALLOW_COPY_AND_ASSIGN(AnnotationsDirectoryItem);
    };

    struct FieldAnnotationsItem
    {
        uint32_t field_idx_;
        uint32_t annotations_off_;

      private:
        DISALLOW_COPY_AND_ASSIGN(FieldAnnotationsItem);
    };

    struct MethodAnnotationsItem
    {
        uint32_t method_idx_;
        uint32_t annotations_off_;

      private:
        DISALLOW_COPY_AND_ASSIGN(MethodAnnotationsItem);
    };

    struct ParameterAnnotationsItem
    {
        uint32_t method_idx_;
        uint32_t annotations_off_;

      private:
        DISALLOW_COPY_AND_ASSIGN(ParameterAnnotationsItem);
    };

    struct AnnotationSetRefItem
    {
        uint32_t annotations_off_;

      private:
        DISALLOW_COPY_AND_ASSIGN(AnnotationSetRefItem);
    };

    struct AnnotationSetRefList
    {
        uint32_t size_;
        AnnotationSetRefItem list_[1];

      private:
        DISALLOW_COPY_AND_ASSIGN(AnnotationSetRefList);
    };

    struct AnnotationSetItem
    {
        uint32_t size_;
        uint32_t entries_[1];

      private:
        DISALLOW_COPY_AND_ASSIGN(AnnotationSetItem);
    };

    struct AnnotationItem
    {
        uint8_t visibility_;
        uint8_t annotation_[1];

      private:
        DISALLOW_COPY_AND_ASSIGN(AnnotationItem);
    };


    ~DexFile();

    // Opens a .dex file at the given memory address.
    static const DexFile* OpenMemory(ScopedMap& mem_map)
    {
        return OpenMemory(mem_map.GetBase(), mem_map.GetSize(), mem_map);
    }

    // Returns true if the byte string points to the magic value.
    static bool IsMagicValid(const byte* magic);

    // Returns true if the byte string after the magic is the correct value.
    static bool IsVersionValid(const byte* magic);

    /*------------------------------------------------------------------*
     *             Functions for string_id_item Manipulation            *
     *------------------------------------------------------------------*/
    // Returns the number of string identifiers in the .dex file.
    size_t NumStringIds() const
    {
        CHECK(header_ != nullptr);
        return header_->string_ids_size_;
    }

    // Returns the StringId at the specified index.
    const StringId& GetStringId(uint32_t idx) const
    {
        CHECK_LT(idx, NumStringIds());
        return string_ids_[idx];
    }

    uint32_t GetIndexForStringId(const StringId& string_id) const
    {
        CHECK_GE(&string_id, string_ids_);
        CHECK_LT(&string_id, string_ids_ + header_->string_ids_size_);
        return &string_id - string_ids_;
    }

    int32_t GetStringLength(const StringId& string_id) const;

    // Returns a pointer to the UTF-8 string data referred to by the given
    // string_id as well as the length of the string when decoded as a UTF-16
    // string. Note the UTF-16 length is not the same as the string length of
    // the string data.
    const char* GetStringDataAndUtf16Length(const StringId& string_id,
                                            uint32_t* utf16_length) const;

    const char* GetStringData(const StringId& string_id) const
    {
        uint32_t ignored;
        return GetStringDataAndUtf16Length(string_id, &ignored);
    }

    // Index version of GetStringDataAndUtf16Length.
    const char* StringDataAndUtf16LengthByIdx(uint32_t idx, uint32_t* utf16_length) const
    {
        if (idx == kDexNoIndex) {
            *utf16_length = 0;
            return nullptr;
        }
        const StringId& string_id = GetStringId(idx);
        return GetStringDataAndUtf16Length(string_id, utf16_length);
    }

    const char* StringDataByIdx(uint32_t idx) const
    {
        uint32_t unicode_length;
        return StringDataAndUtf16LengthByIdx(idx, &unicode_length);
    }


    /*------------------------------------------------------------------*
     *              Functions for type_id_item Manipulation             *
     *------------------------------------------------------------------*/
    // Returns the number of type identifiers in the .dex file.
    uint32_t NumTypeIds() const
    {
        CHECK(header_ != nullptr);
        return header_->type_ids_size_;
    }

    // Returns the TypeId at the specified index.
    const TypeId& GetTypeId(uint32_t idx) const
    {
        CHECK_LT(idx, NumTypeIds());
        return type_ids_[idx];
    }

    uint16_t GetIndexForTypeId(const TypeId& type_id) const
    {
        CHECK_GE(&type_id, type_ids_);
        CHECK_LT(&type_id, type_ids_ + header_->type_ids_size_);
        size_t result = &type_id - type_ids_;
        CHECK_LT(result, 65536U);
        return static_cast<uint16_t>(result);
    }

    // Get the descriptor string associated with a given type index.
    const char* StringByTypeIdx(uint32_t idx, uint32_t* unicode_length) const
    {
        const TypeId& type_id = GetTypeId(idx);
        return StringDataAndUtf16LengthByIdx(type_id.descriptor_idx_, unicode_length);
    }

    const char* StringByTypeIdx(uint32_t idx) const
    {
        const TypeId& type_id = GetTypeId(idx);
        return StringDataByIdx(type_id.descriptor_idx_);
    }

    // Returns the type descriptor string of a type id.
    const char* GetTypeDescriptor(const TypeId& type_id) const
    {
        return StringDataByIdx(type_id.descriptor_idx_);
    }


    /*------------------------------------------------------------------*
     *             Functions for field_id_item Manipulation             *
     *------------------------------------------------------------------*/
    // Returns the number of field identifiers in the .dex file.
    size_t NumFieldIds() const
    {
        CHECK(header_ != nullptr);
        return header_->field_ids_size_;
    }

    // Returns the FieldId at the specified index.
    const FieldId& GetFieldId(uint32_t idx) const
    {
        CHECK_LT(idx, NumFieldIds());
        return field_ids_[idx];
    }

    uint32_t GetIndexForFieldId(const FieldId& field_id) const
    {
        CHECK_GE(&field_id, field_ids_);
        CHECK_LT(&field_id, field_ids_ + header_->field_ids_size_);
        return &field_id - field_ids_;
    }

    // Returns the declaring class descriptor string of a field id.
    const char* GetFieldDeclaringClassDescriptor(const FieldId& field_id) const
    {
        const DexFile::TypeId& type_id = GetTypeId(field_id.class_idx_);
        return GetTypeDescriptor(type_id);
    }

    // Returns the class descriptor string of a field id.
    const char* GetFieldTypeDescriptor(const FieldId& field_id) const
    {
        const DexFile::TypeId& type_id = GetTypeId(field_id.type_idx_);
        return GetTypeDescriptor(type_id);
    }

    // Returns the name of a field id.
    const char* GetFieldName(const FieldId& field_id) const
    {
        return StringDataByIdx(field_id.name_idx_);
    }

    /*------------------------------------------------------------------*
     *             Functions for method_id_item Manipulation            *
     *------------------------------------------------------------------*/
    // Returns the number of method identifiers in the .dex file.
    size_t NumMethodIds() const
    {
        CHECK(header_ != nullptr);
        return header_->method_ids_size_;
    }

    // Returns the MethodId at the specified index.
    const MethodId& GetMethodId(uint32_t idx) const
    {
        CHECK_LT(idx, NumMethodIds());
        return method_ids_[idx];
    }

    uint32_t GetIndexForMethodId(const MethodId& method_id) const
    {
        CHECK_GE(&method_id, method_ids_);
        CHECK_LT(&method_id, method_ids_ + header_->method_ids_size_);
        return &method_id - method_ids_;
    }

    // Returns the declaring class descriptor string of a method id.
    const char* GetMethodDeclaringClassDescriptor(const MethodId& method_id) const
    {
        const DexFile::TypeId& type_id = GetTypeId(method_id.class_idx_);
        return GetTypeDescriptor(type_id);
    }

    // Returns the prototype of a method id.
    const ProtoId& GetMethodPrototype(const MethodId& method_id) const
    {
        return GetProtoId(method_id.proto_idx_);
    }

    const Signature GetMethodSignature(const MethodId& method_id) const;

    // Returns the name of a method id.
    const char* GetMethodName(const MethodId& method_id) const
    {
        return StringDataByIdx(method_id.name_idx_);
    }

    // Returns the shorty of a method id.
    const char* GetMethodShorty(const MethodId& method_id) const
    {
        return StringDataByIdx(GetProtoId(method_id.proto_idx_).shorty_idx_);
    }

    const char* GetMethodShorty(const MethodId& method_id, uint32_t* length) const
    {
        // Using the UTF16 length is safe here as shorties are guaranteed to be ASCII characters.
        return StringDataAndUtf16LengthByIdx(GetProtoId(method_id.proto_idx_).shorty_idx_, length);
    }


    /*------------------------------------------------------------------*
     *            Functions for class_def_item Manipulation             *
     *------------------------------------------------------------------*/
    // Returns the number of class definitions in the .dex file.
    uint32_t NumClassDefs() const
    {
        CHECK(header_ != nullptr);
        return header_->class_defs_size_;
    }

    // Returns the ClassDef at the specified index.
    const ClassDef& GetClassDef(uint16_t idx) const
    {
        CHECK_LT(idx, NumClassDefs());
        return class_defs_[idx];
    }

    uint16_t GetIndexForClassDef(const ClassDef& class_def) const
    {
        CHECK_GE(&class_def, class_defs_);
        CHECK_LT(&class_def, class_defs_ + header_->class_defs_size_);
        return &class_def - class_defs_;
    }

    // Returns the class descriptor string of a class definition.
    const char* GetClassDescriptor(const ClassDef& class_def) const
    {
        return StringByTypeIdx(class_def.class_idx_);
    }

    const TypeList* GetInterfacesList(const ClassDef& class_def) const
    {
        if (class_def.interfaces_off_ == 0)
            return nullptr;
        else {
            const byte* addr = begin_ + class_def.interfaces_off_;
            return reinterpret_cast<const TypeList*>(addr);
        }
    }


    /*------------------------------------------------------------------*
     *    Functions for class_data_item and code_item Manipulation      *
     *------------------------------------------------------------------*/
    // Returns a pointer to the raw memory mapped class_data_item
    const byte* GetClassData(const ClassDef& class_def) const
    {
        if (class_def.class_data_off_ == 0)
            return nullptr;
        else
            return begin_ + class_def.class_data_off_;
    }

    const CodeItem* GetCodeItem(const uint32_t code_off) const
    {
        if (code_off == 0)
            return nullptr;  // native or abstract method
        else {
            const byte* addr = begin_ + code_off;
            return reinterpret_cast<const CodeItem*>(addr);
        }
    }


    /*------------------------------------------------------------------*
     *             Functions for proto_id_item Manipulation             *
     *------------------------------------------------------------------*/
    const char* GetReturnTypeDescriptor(const ProtoId& proto_id) const
    {
        return StringByTypeIdx(proto_id.return_type_idx_);
    }

    // Returns the number of prototype identifiers in the .dex file.
    size_t NumProtoIds() const
    {
        CHECK(header_ != nullptr);
        return header_->proto_ids_size_;
    }

    // Returns the ProtoId at the specified index.
    const ProtoId& GetProtoId(uint32_t idx) const
    {
        CHECK_LT(idx, NumProtoIds());
        return proto_ids_[idx];
    }

    uint16_t GetIndexForProtoId(const ProtoId& proto_id) const
    {
        CHECK_GE(&proto_id, proto_ids_);
        CHECK_LT(&proto_id, proto_ids_ + header_->proto_ids_size_);
        return &proto_id - proto_ids_;
    }

    // Returns the short form method descriptor for the given prototype.
    const char* GetShorty(uint32_t proto_idx) const
    {
        const ProtoId& proto_id = GetProtoId(proto_idx);
        return StringDataByIdx(proto_id.shorty_idx_);
    }

    const TypeList* GetProtoParameters(const ProtoId& proto_id) const
    {
        if (proto_id.parameters_off_ == 0)
            return nullptr;
        else {
            const byte* addr = begin_ + proto_id.parameters_off_;
            return reinterpret_cast<const TypeList*>(addr);
        }
    }

  private:

    static const DexFile* OpenMemory(byte* base, size_t size, ScopedMap& mem_map);

    DexFile(byte* base, size_t size, ScopedMap& mem_map);

    // The base address of the memory mapping.
    const byte* const begin_;

    // The size of the underlying memory allocation in bytes.
    const size_t size_;

    // Manages the underlying memory allocation.
    ScopedMap mem_map_;

    // Points to the header section.
    const Header* const header_;

    // Points to the base of the string identifier list.
    const StringId* const string_ids_;

    // Points to the base of the type identifier list.
    const TypeId* const type_ids_;

    // Points to the base of the field identifier list.
    const FieldId* const field_ids_;

    // Points to the base of the method identifier list.
    const MethodId* const method_ids_;

    // Points to the base of the prototype identifier list.
    const ProtoId* const proto_ids_;

    // Points to the base of the class definition list.
    const ClassDef* const class_defs_;

};


// Abstract the signature of a method.
class Signature
{
  public:
    std::string ToString() const;

    static Signature NoSignature()
    {
        return Signature();
    }

    bool operator==(const Signature& rhs) const;
    bool operator!=(const Signature& rhs) const
    {
        return !(*this == rhs);
    }

    bool operator==(const StringPiece& rhs) const;

  private:
    Signature(const DexFile* dex, const DexFile::ProtoId& proto)
      : dex_file_(dex), proto_id_(&proto)
    {}

    Signature()
      : dex_file_(nullptr), proto_id_(nullptr)
    {}

    friend class DexFile;

    const DexFile* const dex_file_;
    const DexFile::ProtoId* const proto_id_;
};

// Iterate and decode class_data_item.
class ClassDataItemIterator
{
  public:
    ClassDataItemIterator(const DexFile& dex_file, const byte* raw_class_data_item)
      : dex_file_(dex_file), pos_(0), ptr_pos_(raw_class_data_item), last_idx_(0)
    {
        ReadClassDataHeader();
        if (EndOfInstanceFieldsPos() > 0)
            ReadClassDataField();
        else if (EndOfVirtualMethodsPos() > 0)
            ReadClassDataMethod();
    }

    uint32_t NumStaticFields() const
    {
        return header_.static_fields_size_;
    }

    uint32_t NumInstanceFields() const
    {
        return header_.instance_fields_size_;
    }

    uint32_t NumDirectMethods() const
    {
        return header_.direct_methods_size_;
    }

    uint32_t NumVirtualMethods() const
    {
        return header_.virtual_methods_size_;
    }

    bool HasNextStaticField() const
    {
        return pos_ < EndOfStaticFieldsPos();
    }

    bool HasNextInstanceField() const
    {
        return pos_ >= EndOfStaticFieldsPos() && pos_ < EndOfInstanceFieldsPos();
    }

    bool HasNextDirectMethod() const
    {
        return pos_ >= EndOfInstanceFieldsPos() && pos_ < EndOfDirectMethodsPos();
    }

    bool HasNextVirtualMethod() const
    {
        return pos_ >= EndOfDirectMethodsPos() && pos_ < EndOfVirtualMethodsPos();
    }

    bool HasNext() const
    {
        return pos_ < EndOfVirtualMethodsPos();
    }

    inline void Next()
    {
        pos_++;
        if (pos_ < EndOfStaticFieldsPos()) {
            last_idx_ = GetMemberIndex();
            ReadClassDataField();
        } else if (pos_ == EndOfStaticFieldsPos() && NumInstanceFields() > 0) {
            last_idx_ = 0;  // transition to next array, reset last index
            ReadClassDataField();
        } else if (pos_ < EndOfInstanceFieldsPos()) {
            last_idx_ = GetMemberIndex();
            ReadClassDataField();
        } else if (pos_ == EndOfInstanceFieldsPos() && NumDirectMethods() > 0) {
            last_idx_ = 0;  // transition to next array, reset last index
            ReadClassDataMethod();
        } else if (pos_ < EndOfDirectMethodsPos()) {
            last_idx_ = GetMemberIndex();
            ReadClassDataMethod();
        } else if (pos_ == EndOfDirectMethodsPos() && NumVirtualMethods() > 0) {
            last_idx_ = 0;  // transition to next array, reset last index
            ReadClassDataMethod();
        } else if (pos_ < EndOfVirtualMethodsPos()) {
            last_idx_ = GetMemberIndex();
            ReadClassDataMethod();
        } else
            CHECK(!HasNext());
    }

    uint32_t GetMemberIndex() const
    {
        if (pos_ < EndOfInstanceFieldsPos())
            return last_idx_ + field_.field_idx_delta_;
        else {
            CHECK_LT(pos_, EndOfVirtualMethodsPos());
            return last_idx_ + method_.method_idx_delta_;
        }
    }

    uint32_t GetRawMemberAccessFlags() const
    {
        if (pos_ < EndOfInstanceFieldsPos()) {
            return field_.access_flags_;
        } else {
            CHECK_LT(pos_, EndOfVirtualMethodsPos());
            return method_.access_flags_;
        }
    }

    uint32_t GetFieldAccessFlags() const
    {
        return GetRawMemberAccessFlags() & kAccValidFieldFlags;
    }

    uint32_t GetMethodAccessFlags() const
    {
        return GetRawMemberAccessFlags() & kAccValidMethodFlags;
    }

    bool MemberIsNative() const
    {
        return GetRawMemberAccessFlags() & kAccNative;
    }

    bool MemberIsFinal() const
    {
        return GetRawMemberAccessFlags() & kAccFinal;
    }

    InvokeType GetMethodInvokeType(const DexFile::ClassDef& class_def) const
    {
        if (HasNextDirectMethod()) {
            if ((GetRawMemberAccessFlags() & kAccStatic) != 0)
                return kStatic;
            else
                return kDirect;
        } else {
            CHECK_EQ(GetRawMemberAccessFlags() & kAccStatic, 0U);
            if ((class_def.access_flags_ & kAccInterface) != 0)
                return kInterface;
            else if ((GetRawMemberAccessFlags() & kAccConstructor) != 0)
                return kSuper;
            else
                return kVirtual;
        }
    }

    const DexFile::CodeItem* GetMethodCodeItem() const
    {
        return dex_file_.GetCodeItem(method_.code_off_);
    }

    uint32_t GetMethodCodeItemOffset() const
    {
        return method_.code_off_;
    }

    const byte* EndDataPointer() const
    {
        CHECK(!HasNext());
        return ptr_pos_;
    }

  private:

    // A dex file's class_data_item is leb128 encoded, this structure holds a
    // decoded form of the header for a class_data_item.
    struct ClassDataHeader
    {
        uint32_t static_fields_size_;  // the number of static fields
        uint32_t instance_fields_size_;  // the number of instance fields
        uint32_t direct_methods_size_;  // the number of direct methods
        uint32_t virtual_methods_size_;  // the number of virtual methods
    } header_;

    // Read and decode header from a class_data_item stream into header.
    void ReadClassDataHeader();

    uint32_t EndOfStaticFieldsPos() const
    {
        return header_.static_fields_size_;
    }

    uint32_t EndOfInstanceFieldsPos() const
    {
        return EndOfStaticFieldsPos() + header_.instance_fields_size_;
    }

    uint32_t EndOfDirectMethodsPos() const
    {
        return EndOfInstanceFieldsPos() + header_.direct_methods_size_;
    }

    uint32_t EndOfVirtualMethodsPos() const
    {
        return EndOfDirectMethodsPos() + header_.virtual_methods_size_;
    }

    // A decoded version of the field of a class_data_item.
    struct ClassDataField
    {
        uint32_t field_idx_delta_;  // delta of index into the field_ids array for FieldId
        uint32_t access_flags_;  // access flags for the field

        ClassDataField()
          : field_idx_delta_(0), access_flags_(0)
        {}

      private:
        DISALLOW_COPY_AND_ASSIGN(ClassDataField);
    };
    ClassDataField field_;

    // Read and decode a field from a class_data_item stream into field
    void ReadClassDataField();

    // A decoded version of the method of a class_data_item.
    struct ClassDataMethod
    {
        uint32_t method_idx_delta_;  // delta of index into the method_ids array for MethodId
        uint32_t access_flags_;
        uint32_t code_off_;

        ClassDataMethod()
          : method_idx_delta_(0), access_flags_(0), code_off_(0)
        {}

      private:
        DISALLOW_COPY_AND_ASSIGN(ClassDataMethod);
    };
    ClassDataMethod method_;

    // Read and decode a method from a class_data_item stream into method
    void ReadClassDataMethod();

    const DexFile& dex_file_;
    size_t pos_;  // integral number of items passed
    const byte* ptr_pos_;  // pointer into stream of class_data_item
    uint32_t last_idx_;  // last read field or method index to apply delta to
    DISALLOW_IMPLICIT_CONSTRUCTORS(ClassDataItemIterator);
};

#endif