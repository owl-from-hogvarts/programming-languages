import t from "tap"
import { spawn } from "node:child_process"

const APP_PATH = "./target/main"

function runApp(path: string, input: string, useStdout = true): Promise<string> {
    return new Promise((resolve, reject) => {
      const cp = spawn(path)
      const { stdin, stdout, stderr } = cp
      const outputStream = useStdout ? stdout : stderr

      outputStream.setEncoding("ascii")

      stdin.write(input + "\n") // \n is to simulate user input
      
      let output = ""
      
      outputStream.on("data", (chunk: string) => {
        output += chunk
      })

      outputStream.once("end", () => {
        resolve(output)
      })
    })
}


t.test("key is present in dictionary", async t => {
  const result = await runApp(APP_PATH, "first word")

  t.equal(result, "first word explanation")

})

t.test("no such key in dictionary", async t => {
  const result = await runApp(APP_PATH, "gfhjkl", false)

  t.equal(result, "Element with provided key not found")
})

t.test("too large input", async t => {
  // max buffer size is 255 bytes
  const result = await runApp(APP_PATH, "0".repeat(256), false)

  t.equal(result, "Invalid input!")
})

// test correct word
// test incorrect word
// test buffer overflow