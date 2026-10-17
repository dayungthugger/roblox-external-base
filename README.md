## roblox-ext

A clean Roblox external written in C++.
DirectComposition overlay · Low latency · D3D11 flip swapchain
<br> </div>

## About

Lightweight Roblox external base.

It finds `RobloxPlayerBeta.exe` via a Toolhelp process snapshot, opens it with `OpenProcess`, and resolves the module base from a module snapshot. Memory is read with direct `NtReadVirtualMemory` / `NtWriteVirtualMemory` syscalls.

The overlay uses a fullscreen borderless popup with `WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE`, a D3D11 device, and a flip-sequential swapchain built for composition. The swapchain is attached through DirectComposition (`CreateTargetForHwnd` + visual + `SetContent`)

## Features

- Boxes (full / corner)
- Health bar
- Names
- Distance (studs to meters, with max distance filter)
- Snaplines (mouse / bottom / top)
- Self, VSync

## Requirements

- Windows 10/11
- Visual Studio 2022+
- Windows SDK