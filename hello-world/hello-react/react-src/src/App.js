import { useEffect } from 'react'
import './App.css';

// Import filesystem namespace
import { filesystem } from "@neutralinojs/lib"

function App() {

  // Log current directory or error after component is mounted
  useEffect(() => {
    filesystem.readDirectory('./').then((data) => {
      console.log(data)
    }).catch((err) => {
      console.log(err)
    })
  }, [])

  return (
    <div className="App">
      My Neutralinojs App
    </div>
  );
}

export default App;