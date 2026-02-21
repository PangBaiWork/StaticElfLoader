
// target cant found, because it is loaded by loader, 
// not by the system, 
// so we need to hook the loader to get the target base address
var loader = Process.findModuleByName("loader");
console.log("loader base: " + loader.base);
Interceptor.attach(loader.findSymbolByName("run_elf"),
    {
        onEnter: function (args) {
            console.log("hooked run_elf")
            /* may need*/ 
            // let target_base = args[0].readPointer();
            // console.log("target base: " + target_base); 
            Interceptor.attach(ptr(0x401C60),
                {
                    onEnter: function (args) {
                        console.log("\nhooked target elf address\n")
                    }
                }
            );
        }
    }
);