object WindowRuleEditForm: TWindowRuleEditForm
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = '���ڹ���༭'
  ClientHeight = 418
  ClientWidth = 434
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 24
    Top = 24
    Width = 24
    Height = 13
    Caption = '����'
  end
  object Label2: TLabel
    Left = 24
    Top = 64
    Width = 24
    Height = 13
    Caption = '����'
  end
  object Label3: TLabel
    Left = 24
    Top = 104
    Width = 48
    Height = 13
    Caption = '���ڱ���'
  end
  object Label4: TLabel
    Left = 24
    Top = 144
    Width = 48
    Height = 13
    Caption = '��������'
  end
  object Label5: TLabel
    Left = 24
    Top = 184
    Width = 48
    Height = 13
    Caption = '������ʽ'
  end
  object Label6: TLabel
    Left = 24
    Top = 224
    Width = 48
    Height = 13
    Caption = '����·��'
  end
  object Label7: TLabel
    Left = 24
    Top = 263
    Width = 48
    Height = 13
    Caption = '��ݼ�'
  end
  object edtName: TEdit
    Left = 88
    Top = 21
    Width = 321
    Height = 21
    TabOrder = 0
  end
  object cbbAction: TComboBox
    Left = 88
    Top = 61
    Width = 321
    Height = 21
    Style = csDropDownList
    ItemIndex = 0
    TabOrder = 1
    Text = 'Ignore'
    Items.Strings = (
      'Ignore'
      'TopMost'
    )
  end
  object edtTitle: TEdit
    Left = 88
    Top = 101
    Width = 321
    Height = 21
    TabOrder = 2
  end
  object edtClassName: TEdit
    Left = 88
    Top = 141
    Width = 321
    Height = 21
    TabOrder = 7
  end
  object edtStyle: TEdit
    Left = 88
    Top = 181
    Width = 321
    Height = 21
    TabOrder = 4
  end
  object edtExePath: TEdit
    Left = 88
    Top = 221
    Width = 321
    Height = 21
    TabOrder = 5
  end
  object edtHotkeyKey: TEdit
    Left = 88
    Top = 260
    Width = 33
    Height = 21
    MaxLength = 1
    TabOrder = 3
    OnKeyPress = edtHotkeyKeyKeyPress
  end
  object Panel1: TPanel
    Left = 0
    Top = 377
    Width = 434
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 6
    ExplicitTop = 240
    object btnOK: TButton
      Left = 256
      Top = 8
      Width = 75
      Height = 25
      Caption = 'ȷ��'
      Default = True
      ModalResult = 1
      TabOrder = 0
      OnClick = btnOKClick
    end
    object btnCancel: TButton
      Left = 337
      Top = 8
      Width = 75
      Height = 25
      Cancel = True
      Caption = 'ȡ��'
      ModalResult = 2
      TabOrder = 1
    end
  end
end
