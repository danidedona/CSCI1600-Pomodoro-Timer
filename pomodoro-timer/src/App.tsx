import { NavLink, Route, Routes } from "react-router-dom";
import Insights from "./components/Insights";
import Settings from "./components/Settings";
import About from "./components/About";
import logo from "./assets/tomato.png";
import "./App.css";
function App() {
  return (
    <>
      <header>
        <nav>
          <NavLink to={"/"} className="left">
            <img src={logo} className="logo"></img>
            Pomo Timer
          </NavLink>
          <div className="right">
            <NavLink to={"/"}>Insights</NavLink>
            <NavLink to={"/settings"}>Settings</NavLink>
            <NavLink to={"/about"}>About</NavLink>
          </div>
        </nav>
      </header>
      <Routes>
        <Route path="/" element={<Insights />} />
        <Route path="/insights" element={<Insights />} />
        <Route path="/settings" element={<Settings />} />
        <Route path="/about" element={<About />} />
      </Routes>
    </>
  );
}

export default App;
