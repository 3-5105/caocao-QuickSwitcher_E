unit HotKeyEdit;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, System.IniFiles;

type
  THotKeyEditForm = class(TForm)
    Label1: TLabel;
    edtName: TEdit;
    Label2: TLabel;
    edtHotKey: TEdit;
    chkFollowMouse: TCheckBox;
    btnOK: TButton;
    btnCancel: TButton;
    Label3: TLabel;
    cbbListType: TComboBox;
    lblListType: TLabel;
    procedure FormCreate(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure btnOKClick(Sender: TObject);
  private
    FOldSection: string;  // ����ɵĽ�������������
    FEditingSection: string;
    FKey: string;
    FCtrl: Boolean;
    FShift: Boolean;
    FAlt: Boolean;
    FWin: Boolean;
    FOnHotKeyChanged: TNotifyEvent;
    FOriginalWndProc: TWndMethod;  // ����ԭʼ�Ĵ��ڹ���
    FKeyDown: Boolean;  // ��ӱ�־λ
    procedure UpdateHotKeyText;
    procedure InitializeControls;
    procedure HotKeyWndProc(var Message: TMessage);  // �Զ��崰�ڹ���
    procedure edtHotKeyKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
  public
    procedure LoadHotKey(const Section: string);
    property OnHotKeyChanged: TNotifyEvent read FOnHotKeyChanged write FOnHotKeyChanged;
    property EditingSection: string read FEditingSection write FEditingSection;  // ����������
  end;

implementation

{$R *.dfm}

function IsKeyDown(VKey: Integer): Boolean;
begin
  Result := (GetKeyState(VKey) and $8000) <> 0;
end;

function GetKeyText(Key: Word): string;
var
  KeyState: TKeyboardState;
  Chars: array[0..1] of Char;
begin
  if Key = 0 then Exit('');
  if Key = VK_LWIN then Exit('Win');
  if Key = VK_RWIN then Exit('Win');
  if Key = VK_RBUTTON then Exit('RButton');
  if Key = VK_MBUTTON then Exit('MButton');
  
  // ��ȡ��������
  SetLength(Result, 30);
  GetKeyNameText(MapVirtualKey(Key, 0) shl 16, PChar(Result), 30);
  Result := PChar(Result);
  
  // �����ȡ�������ƣ�����ת��Ϊ�ַ�
  if Result = '' then
  begin
    GetKeyboardState(KeyState);
    if ToAscii(Key, MapVirtualKey(Key, 0), KeyState, @Chars, 0) = 1 then
      Result := UpperCase(Chars[0]);
  end;
end;

procedure THotKeyEditForm.FormCreate(Sender: TObject);
begin
  inherited;  // ������У�ȷ�������FormCreate��ִ��
  
  FOldSection := '';
  FEditingSection := '';
  FKey := '';
  FCtrl := False;
  FAlt := False;
  FWin := False;
  FShift := False;  // ��������֮ǰ©����
  FKeyDown := False;  // ��ӱ�־λ
  
  edtHotKey.Text := '�밴�¿�ݼ�...';
  
  // ����ԭʼ�Ĵ��ڹ��̲��滻Ϊ�Զ����
  FOriginalWndProc := edtHotKey.WindowProc;
  edtHotKey.WindowProc := HotKeyWndProc;
end;

procedure THotKeyEditForm.InitializeControls;
begin
  // ��ʼ��ListType������
  cbbListType.Items.Clear;
  cbbListType.Items.Add('��״��ͼ');   // ListType = 0
  cbbListType.Items.Add('����˵�');   // ListType = 1
  // ItemIndex����LoadHotKey�и��ݶ�ȡ��ֵ����
end;

procedure THotKeyEditForm.FormShow(Sender: TObject);
begin
  InitializeControls;
  if FEditingSection <> '' then
    LoadHotKey(FEditingSection);
end;

procedure THotKeyEditForm.UpdateHotKeyText;
var
  KeyText: string;
begin
  KeyText := '';
  if FCtrl then KeyText := KeyText + 'Ctrl + ';
  if FShift then KeyText := KeyText + 'Shift + ';
  if FAlt then KeyText := KeyText + 'Alt + ';
  if FWin then KeyText := KeyText + 'Win + ';
  
  if FKey <> '' then
    KeyText := KeyText + FKey
  else if KeyText <> '' then
  begin
    // �Ƴ�ĩβ�� ' + '
    if KeyText.EndsWith(' + ') then
      SetLength(KeyText, Length(KeyText) - 3);
  end;
    
  if KeyText = '' then
    KeyText := '�밴�¿�ݼ�...';
    
  edtHotKey.Text := '';
  edtHotKey.Text := KeyText;
end;

procedure THotKeyEditForm.LoadHotKey(const Section: string);
var
  Ini: TIniFile;
  KeyChar: string;
  ListType: Integer;  // ���ListType����
begin
  FOldSection := Section;  // ����ɵĽ���
  FEditingSection := Section;
  
  Ini := TIniFile.Create(ExtractFilePath(Application.ExeName) + 'UiSetting.ini');
  try
    if Section <> '' then
    begin
      edtName.Text := Section.Substring(7);  // �Ƴ� 'HotKey.' ǰ׺
      FCtrl := Ini.ReadBool(Section, 'ctrl', False);
      FShift := Ini.ReadBool(Section, 'shift', False);
      FAlt := Ini.ReadBool(Section, 'alt', False);
      FWin := Ini.ReadBool(Section, 'win', False);
      KeyChar := Ini.ReadString(Section, 'key', '');
      if KeyChar <> '' then
        FKey := KeyChar;
      chkFollowMouse.Checked := Ini.ReadBool(Section, 'followMouse', False);
      
      // ��ȡListType����
      ListType := Ini.ReadInteger(Section, 'ListType', 0);
      cbbListType.ItemIndex := ListType;  // ����������ĳ�ʼֵ
    end;
    UpdateHotKeyText;
  finally
    Ini.Free;
  end;
end;

procedure THotKeyEditForm.btnOKClick(Sender: TObject);
var
  Ini: TIniFile;
  NewSection, KeyChar: string;
begin
  if Trim(edtName.Text) = '' then
  begin
    ShowMessage('���������ƣ�');
    edtName.SetFocus;
    Exit;
  end;
  
  if Trim(edtHotKey.Text) = '' then
  begin
    ShowMessage('�����ÿ�ݼ���');
    edtHotKey.SetFocus;
    Exit;
  end;

  NewSection := 'HotKey.' + Trim(edtName.Text);
  
  Ini := TIniFile.Create(ExtractFilePath(Application.ExeName) + 'UiSetting.ini');
  try
    // ��������ı��ˣ�ɾ���ɵĽ�
    if (FOldSection <> '') and (FOldSection <> NewSection) then
      Ini.EraseSection(FOldSection);
      
    // д���µ�����
    Ini.WriteBool(NewSection, 'ctrl', FCtrl);
    Ini.WriteBool(NewSection, 'shift', FShift);
    Ini.WriteBool(NewSection, 'alt', FAlt);
    Ini.WriteBool(NewSection, 'win', FWin);
    Ini.WriteString(NewSection, 'key', FKey);
    Ini.WriteBool(NewSection, 'followMouse', chkFollowMouse.Checked);
    
    // ����ListType����
    Ini.WriteInteger(NewSection, 'ListType', cbbListType.ItemIndex);
  finally
    Ini.Free;
  end;
  
  if Assigned(FOnHotKeyChanged) then
    FOnHotKeyChanged(Self);
    
  ModalResult := mrOk;
end;

procedure THotKeyEditForm.HotKeyWndProc(var Message: TMessage);
var
  Key: Word;
  Shift: TShiftState;
  RepeatCount: Integer;
  PrevKeyState: Boolean;
begin

  // ������̺������Ϣ
  case Message.Msg of
    WM_KEYDOWN, WM_SYSKEYDOWN:
      begin
        // ��ȡ�ظ�������lParam�ĵ�16λ��
        RepeatCount := Message.LParam and $FFFF;
        // ��ȡ��һ����״̬��lParam�ĵ�30λ��
        PrevKeyState := (Message.LParam and (1 shl 30)) <> 0;

        // ������ظ��������߼��Ѿ����ڰ���״̬�����
        if (RepeatCount > 1) or PrevKeyState then Exit;

        Key := TWMKey(Message).CharCode;
        Shift := KeyDataToShiftState(TWMKey(Message).KeyData);
        
        // ���ñ�ע�͵����߼�
        FKey := '';
        FCtrl := False;
        FShift := False;
        FAlt := False;
        FWin := False;

        if ssCtrl in Shift then FCtrl := True;
        if ssShift in Shift then FShift := True;
        if ssAlt in Shift then FAlt := True;
        if (Key = VK_LWIN) or (Key = VK_RWIN) then FWin := True;

        if not (Key in [VK_CONTROL, VK_SHIFT, VK_MENU, VK_LWIN, VK_RWIN]) then
          FKey := GetKeyText(Key);

        UpdateHotKeyText;
        Message.Result := 0;
        Exit;
      end;
    WM_RBUTTONDOWN:
      begin
        Key := VK_RBUTTON;
        edtHotKeyKeyDown(Self, Key, []);
        Message.Result := 0;
        Exit;
      end;
    WM_MBUTTONDOWN:
      begin
        Key := VK_MBUTTON;
        edtHotKeyKeyDown(Self, Key, []);
        Message.Result := 0;
        Exit;
      end;
    WM_MBUTTONUP:
      begin
        FKeyDown := False;
        Message.Result := 0;
        Exit;
      end;
    WM_RBUTTONUP:
      begin
        FKeyDown := False;
        Message.Result := 0;
        Exit;
      end;
  end;

  // ������Ϣ����ԭʼ���ڹ��̴���
  FOriginalWndProc(Message);
end;

procedure THotKeyEditForm.edtHotKeyKeyDown(Sender: TObject; var Key: Word; Shift: TShiftState);
var
  i: Integer;
  HotKey: string;
begin
  // ���������״̬
  FKey := '';
  FCtrl := False;
  FShift := False;
  FAlt := False;
  FWin := False;

  // ����������п��ܵİ���
  for i := 1 to 255 do
  begin
    if IsKeyDown(i) then
    begin
      HotKey := GetKeyText(i);

      if HotKey = 'Ctrl' then FCtrl := True
      else if HotKey = 'Shift' then FShift := True
      else if HotKey = 'Alt' then FAlt := True
      else if HotKey = 'Win' then FWin := True
      else if (FKey = '')  then FKey := HotKey;
    end;
  end;

  UpdateHotKeyText;  // ������ʾ��ǰ���а��µļ�
  Key := 0; // ��ֹĬ�ϴ���
end;

end.