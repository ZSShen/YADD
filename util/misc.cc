
#include "globals.h"
#include "stringprintf.h"
#include "misc.h"
#include "utf.h"
#include "dex_file-inl.h"
#include "utf-inl.h"


std::string PrintableString(const char* utf)
{
    std::string result;
    result += '"';
    const char* p = utf;
    size_t char_count = CountModifiedUtf8Chars(p);
    for (size_t i = 0; i < char_count; ++i) {
        uint16_t ch = GetUtf16FromUtf8(&p);
        if (ch == '\\')
            result += "\\\\";
        else if (ch == '\n')
            result += "\\n";
        else if (ch == '\r')
            result += "\\r";
        else if (ch == '\t')
            result += "\\t";
        else if (NeedsEscaping(ch))
            StringAppendF(&result, "\\u%04x", ch);
        else
            result += ch;
    }
    result += '"';
    return result;
}

std::string PrettyDescriptor(const char* descriptor)
{
    // Count the number of '['s to get the dimensionality.
    const char* c = descriptor;
    size_t dim = 0;
    while (*c == '[') {
        dim++;
        c++;
    }

    // Reference or primitive?
    if (*c == 'L') {
        // "[[La/b/C;" -> "a.b.C[][]".
        c++;  // Skip the 'L'.
    } else {
        // "[[B" -> "byte[][]".
        // To make life easier, we make primitives look like unqualified
        // reference types.
        switch (*c) {
          case 'B': c = "byte;"; break;
          case 'C': c = "char;"; break;
          case 'D': c = "double;"; break;
          case 'F': c = "float;"; break;
          case 'I': c = "int;"; break;
          case 'J': c = "long;"; break;
          case 'S': c = "short;"; break;
          case 'Z': c = "boolean;"; break;
          case 'V': c = "void;"; break;  // Used when decoding return types.
          default: return descriptor;
        }
    }

    // At this point, 'c' is a string of the form "fully/qualified/Type;"
    // or "primitive;". Rewrite the type with '.' instead of '/':
    std::string result;
    const char* p = c;
    while (*p != ';') {
        char ch = *p++;
        if (ch == '/')
            ch = '.';
        result.push_back(ch);
    }
    // ...and replace the semicolon with 'dim' "[]" pairs:
    for (size_t i = 0; i < dim; ++i)
        result += "[]";
    return result;
}

std::string PrettyField(uint32_t field_idx, const DexFile& dex_file, bool with_type)
{
    if (field_idx >= dex_file.NumFieldIds())
        return StringPrintf("<<invalid-field-idx-%d>>", field_idx);

    const DexFile::FieldId& field_id = dex_file.GetFieldId(field_idx);
    std::string result;
    if (with_type) {
        result += dex_file.GetFieldTypeDescriptor(field_id);
        result += ' ';
    }
    result += PrettyDescriptor(dex_file.GetFieldDeclaringClassDescriptor(field_id));
    result += '.';
    result += dex_file.GetFieldName(field_id);
    return result;
}

std::string PrettyArguments(const char* signature)
{
    std::string result;
    result += '(';
    assert(*signature == '(');
    ++signature;  // Skip the '('.
    while (*signature != ')') {
        size_t argument_length = 0;
        while (signature[argument_length] == '[')
            ++argument_length;
        if (signature[argument_length] == 'L')
            argument_length = (strchr(signature, ';') - signature + 1);
        else
            ++argument_length;
        {
            std::string argument_descriptor(signature, argument_length);
            result += PrettyDescriptor(argument_descriptor.c_str());
        }
        if (signature[argument_length] != ')')
            result += ", ";
        signature += argument_length;
    }
    assert(*signature == ')');
    ++signature;  // Skip the ')'.
    result += ')';
    return result;
}

std::string PrettyReturnType(const char* signature)
{
    const char* return_type = strchr(signature, ')');
    assert(return_type != NULL);
    ++return_type;  // Skip ')'.
    return PrettyDescriptor(return_type);
}

std::string PrettyMethod(uint32_t method_idx, const DexFile& dex_file, bool with_signature)
{
    if (method_idx >= dex_file.NumMethodIds())
        return StringPrintf("<<invalid-method-idx-%d>>", method_idx);

    const DexFile::MethodId& method_id = dex_file.GetMethodId(method_idx);
    std::string result(PrettyDescriptor(dex_file.GetMethodDeclaringClassDescriptor(method_id)));
    result += '.';
    result += dex_file.GetMethodName(method_id);
    if (with_signature) {
        const Signature signature = dex_file.GetMethodSignature(method_id);
        std::string sig_as_string(signature.ToString());
        if (signature == Signature::NoSignature())
            return result + sig_as_string;
        result = PrettyReturnType(sig_as_string.c_str()) + " " + result +
        PrettyArguments(sig_as_string.c_str());
    }
    return result;
}

std::string PrettyType(uint32_t type_idx, const DexFile& dex_file)
{
    if (type_idx >= dex_file.NumTypeIds())
        return StringPrintf("<<invalid-type-idx-%d>>", type_idx);
    const DexFile::TypeId& type_id = dex_file.GetTypeId(type_idx);
    return PrettyDescriptor(dex_file.GetTypeDescriptor(type_id));
}

