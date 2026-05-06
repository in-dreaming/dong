PS D:\mix\agents\game\gweb\dong> zig build examples -Dtarget=x86_64-windows-msvc
examples
└─ install performance_demo
   └─ compile exe performance_demo Debug x86_64-windows-msvc
      └─ compile lib quickjs Debug x86_64-windows-msvc 3 errors
D:\mix\agents\game\gweb\dong\third_party/quickjs/libregexp.c:3190:17: error: call to undeclared library function 'alloca' with type 'void *(unsigned long long)'; ISO C99 and later do not support implicit function declarations
    stack_buf = alloca(alloca_size);
                ^
D:\mix\agents\game\gweb\dong\third_party/quickjs/libregexp.c:3190:17: note: include the header <stdlib.h> or explicitly provide a declaration for 'alloca'
    stack_buf = alloca(alloca_size);
                ^
D:\mix\agents\game\gweb\dong\third_party/quickjs/dtoa.c:31:10: error: 'sys/time.h' file not found
#include <sys/time.h>
         ^~~~~~~~~~~~~
D:\mix\agents\game\gweb\dong\third_party/quickjs/quickjs.c:31:10: error: 'sys/time.h' file not found
#include <sys/time.h>
         ^~~~~~~~~~~~~