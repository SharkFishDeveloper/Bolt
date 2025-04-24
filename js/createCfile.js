const readline = require('readline');
const fs = require("fs");
const path = require("path");

function createCfile() {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });

    rl.question('Enter the name of the C file: ', (filename) => {
        if(fs.existsSync(`./src/${filename}.c`) || fs.existsSync(`./include/${filename}.h`)){
            console.log("Already exists")
        }else{
            fs.writeFileSync(`./src/${filename}.c`,`
#include "${filename}.h"                
                `);
            fs.writeFileSync(`./include/${filename}.h`,`
#ifndef ${filename.toUpperCase()}_H
#define ${filename.toUpperCase()}_H


#endif
            `);
        }
        rl.close();
    });
}

createCfile();