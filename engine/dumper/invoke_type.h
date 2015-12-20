
#ifndef _ART_INVOKE_TYPE_H_
#define _ART_INVOKE_TYPE_H_


enum InvokeType
{
  	kStatic,     // <<static>>
  	kDirect,     // <<direct>>
  	kVirtual,    // <<virtual>>
  	kSuper,      // <<super>>
  	kInterface,  // <<interface>>
  	kMaxInvokeType = kInterface
};

#endif
