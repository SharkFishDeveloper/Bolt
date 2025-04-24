const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');

function getCFiles(dir, fileList = []) {
  const files = fs.readdirSync(dir);
  for (const file of files) {
    const fullPath = path.join(dir, file);
    const stat = fs.statSync(fullPath);
    if (stat.isDirectory()) {
      getCFiles(fullPath, fileList);
    } else if (file.endsWith('.c')) {
      fileList.push(fullPath);
    }
  }
  return fileList;
}

function compileCProject() {
    const srcPath = path.join(__dirname, '../', 'src');
    const cFiles = getCFiles(srcPath);
    if (cFiles.length === 0) {
      console.error('❌ No .c files found in src/');
      return;
    }
  
    const output = 'output.exe';
    const command = `gcc ${cFiles.join(' ')} -I"${srcPath}" -o ${output} -lm`;
  
    console.log(`🚧 Compiling with: ${command}`);
    exec(command, (error, stdout, stderr) => {
      if (error) {
        console.error(`❌ Compilation failed:\n${stderr}`);
      } else {
        console.log('✅ Compilation successful.');
        if (stdout) console.log(stdout);
  
        console.log('\n🏃 Running output.exe...\n');
        exec(`./${output}`, (runErr, runStdout, runStderr) => {
          if (runErr) {
            console.error(`❌ Runtime error:\n${runStderr}`);
          } else {
            console.log(`💬 Program Output:\n${runStdout}`);
          }
        });
      }
    });
  }

compileCProject();
