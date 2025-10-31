flowchart TD
    Start([START]) --> Setup[setup]
    
    Setup --> InitDebug[DEBUG_BEGIN]
    InitDebug --> InitBattery[battMonitor.begin]
    InitBattery --> InitDisplay[u8g2.begin]
    InitDisplay --> SplashScreen[Affichage Splash Screen]
    SplashScreen --> InitUI[ui_init]
    InitUI --> InitTouch[touchManager.begin]
    InitTouch --> InitLED[ledManager.begin]
    InitLED --> InitSound[soundManager.begin]
    InitSound --> LEDTest[Test chenillard LEDs]
    LEDTest --> LoadEEPROM[eeprom.charger]
    LoadEEPROM --> ApplySettings[Appliquer paramètres UI]
    ApplySettings --> SetReady[systemReady = true]
    SetReady --> Loop
    
    Loop([LOOP]) --> CheckReady{systemReady?}
    CheckReady -->|Non| Wait[delay 100ms]
    Wait --> Loop
    
    CheckReady -->|Oui| UpdateBattery[battMonitor.loop]
    UpdateBattery --> UpdateMIDI[MidiManager.read]
    UpdateMIDI --> UpdateVolume[soundManager.readVolumePot]
    UpdateVolume --> UpdateArp[arpeg.tickAdvanced]
    UpdateArp --> DetectEvents[detect_events]
    
    DetectEvents --> ReadEncoder[rotaryEncoder.read]
    ReadEncoder --> ReadButton[digitalRead BUTTON]
    ReadButton --> CheckState{currentState?}
    
    CheckState -->|STATE_MENU| MenuState[État Menu]
    CheckState -->|STATE_GAME| GameState[État Jeu]
    CheckState -->|STATE_GAME_OVER| GameOverState[État Game Over]
    
    MenuState --> HandleMenuEvents[handle_menu_events]
    HandleMenuEvents --> CheckRotary{rotary_event?}
    CheckRotary -->|CW| NextField[ui_nextField]
    CheckRotary -->|CCW| PrevField[ui_prevField]
    CheckRotary -->|None| CheckPush{push_event?}
    
    NextField --> Redraw1[is_redraw = 1]
    PrevField --> Redraw1
    
    CheckPush -->|Yes| CheckForm{ui_getCurrentFormId?}
    CheckForm -->|Form 10 Game| CheckPos10{Position?}
    CheckPos10 -->|Pos 3| StartGame[startSimonGame]
    CheckPos10 -->|Autres| UISelect1[ui_sendSelect]
    
    CheckForm -->|Form 14 Settings| CheckPos14{Position?}
    CheckPos14 -->|Pos 4| SaveSettings[getAllValues + eeprom.sauvegarder]
    CheckPos14 -->|Autres| UISelect2[ui_sendSelect]
    
    CheckForm -->|Autres| UISelect3[ui_sendSelect]
    
    UISelect1 --> Redraw2[is_redraw = 1]
    UISelect2 --> Redraw2
    UISelect3 --> Redraw2
    SaveSettings --> Redraw2
    
    Redraw1 --> UpdateDisplay[updateDisplay]
    Redraw2 --> UpdateDisplay
    
    UpdateDisplay --> CheckFormSettings{Form actif?}
    CheckFormSettings -->|Form 12 Synth| EnableTouch[touchManager.enableTouch]
    CheckFormSettings -->|Form 14| CheckVarChanges[Vérifier changements variables]
    CheckFormSettings -->|Form 15| CheckFXChanges[Vérifier effets]
    CheckFormSettings -->|Form 17| CheckArpChanges[Vérifier arpégiateur]
    
    CheckVarChanges --> ApplyIntensity[ledManager.setIntensity]
    ApplyIntensity --> ApplyVolume[soundManager.volume]
    
    CheckFXChanges --> ApplyReverb[soundManager.setDryWetMix]
    ApplyReverb --> ApplyTremolo[soundManager.setTremoloLevel]
    ApplyTremolo --> ApplyChorus[soundManager.setChorusLevel]
    
    CheckArpChanges --> UpdateArpSettings[arpeg.setMode/setStepInterval]
    
    EnableTouch --> TouchUpdate
    ApplyVolume --> TouchUpdate
    ApplyChorus --> TouchUpdate
    UpdateArpSettings --> TouchUpdate
    
    StartGame --> InitGame[gameLevel = 0<br/>playerInput = 0]
    InitGame --> GetGameSettings[ui_getDifficulty<br/>ui_getNumJoueur<br/>ui_getSpeed]
    GetGameSettings --> CheckDiff[checkDifficulty]
    CheckDiff --> CheckSpd[checkSpeed]
    CheckSpd --> GenSequence[Générer gameSequence random]
    GenSequence --> DisplayGame[displayGameScreen]
    DisplayGame --> StartSeq[playSequence true]
    
    GameState --> PlaySeq[playSequence]
    PlaySeq --> CheckSeqState{seqState?}
    
    CheckSeqState -->|SEQ_WAIT_START| WaitStart[Attendre gameTiming+400ms]
    WaitStart --> SetShowStep[seqState = SEQ_SHOW_STEP]
    
    CheckSeqState -->|SEQ_SHOW_STEP| ShowNote[Afficher LED + Son]
    ShowNote --> PlaySound[soundManager.playTouchSound]
    PlaySound --> SetWaitEnd[seqState = SEQ_WAIT_END]
    
    CheckSeqState -->|SEQ_WAIT_END| WaitEnd[Attendre gameTiming]
    WaitEnd --> StopNote[ledManager + soundManager stop]
    StopNote --> SetPause[seqState = SEQ_PAUSE_BETWEEN]
    
    CheckSeqState -->|SEQ_PAUSE_BETWEEN| WaitPause[Attendre 100ms]
    WaitPause --> IncIndex[seqIndex++]
    IncIndex --> CheckEnd{seqIndex > gameLevel?}
    CheckEnd -->|Non| BackShow[seqState = SEQ_SHOW_STEP]
    CheckEnd -->|Oui| EnableInput[touchManager.enableTouch]
    EnableInput --> DisplayTurn[displayPlayerTurn]
    DisplayTurn --> SetIdle[seqState = SEQ_IDLE]
    
    BackShow --> TouchUpdate
    SetIdle --> TouchUpdate
    SetShowStep --> TouchUpdate
    
    GameOverState --> CheckHighScore{newHighScore?}
    CheckHighScore -->|Oui| ShowHighScore[displayHighScore]
    CheckHighScore -->|Non| CheckRotaryGO{rotary_event?}
    
    ShowHighScore --> CheckRotaryGO
    CheckRotaryGO -->|CW| NextPage[nextPage]
    CheckRotaryGO -->|CCW| PrevPage[prevPage]
    CheckRotaryGO -->|None| CheckPushGO{push_event?}
    
    NextPage --> DisplayPage[displayScorePage]
    PrevPage --> DisplayPage
    
    CheckPushGO -->|Yes| ReturnMenu[returnToMenu]
    ReturnMenu --> NewGame[newGame - reset scores]
    NewGame --> TouchUpdate
    
    DisplayPage --> TouchUpdate
    
    TouchUpdate[touchManager.update] --> CheckTouchEnabled{touchEnabled?}
    CheckTouchEnabled -->|Non| UpdateLEDs
    CheckTouchEnabled -->|Oui| ScanTouch[Scanner touches 0-11]
    
    ScanTouch --> CheckPress{Touche pressée?}
    CheckPress -->|Oui| HandlePress[handleTouchPress]
    CheckPress -->|Non| CheckRelease{Touche relâchée?}
    
    HandlePress --> GetPressure[touchManager.getPressure]
    GetPressure --> GetVelocity[touchManager.getVelocityMs]
    GetVelocity --> ArpNoteOn[arpeg.noteOn]
    ArpNoteOn --> SetLED[ledManager.setLedColor]
    SetLED --> CheckArpEnabled{arpeg.isEnabled?}
    CheckArpEnabled -->|Non| PlayTouch[soundManager.playTouchSound]
    CheckArpEnabled -->|Oui| SkipPlay
    PlayTouch --> SkipPlay
    SkipPlay --> UpdateLEDs
    
    CheckRelease -->|Oui| HandleRelease[handleTouchRelease]
    CheckRelease -->|Non| UpdateLEDs
    
    HandleRelease --> ArpNoteOff[arpeg.noteOff]
    ArpNoteOff --> ClearLED[ledManager.setLedColor BLACK]
    ClearLED --> CheckArpEnabledRel{arpeg.isEnabled?}
    CheckArpEnabledRel -->|Non| StopSound[soundManager.stopOneNote]
    CheckArpEnabledRel -->|Oui| SkipStop
    StopSound --> CheckGameState{currentState = GAME?}
    SkipStop --> CheckGameState
    
    CheckGameState -->|Oui| HandleInput[handleGameInput]
    CheckGameState -->|Non| UpdateLEDs
    
    HandleInput --> CheckCorrect{Bonne touche?}
    CheckCorrect -->|Oui| IncPlayerInput[playerInput++]
    IncPlayerInput --> CheckComplete{Séquence complète?}
    CheckComplete -->|Oui| IncLevel[gameLevel++]
    IncLevel --> CheckMilestone{gameLevel % 10 = 0?}
    CheckMilestone -->|Oui| GameWon[gameWon]
    CheckMilestone -->|Non| NextLevel[playSequence true]
    GameWon --> CheckMax{gameLevel >= 100?}
    CheckMax -->|Oui| GameFinished[gameFinished]
    CheckMax -->|Non| NextLevel
    
    CheckCorrect -->|Non| PlayerLost[currentScore = gameLevel]
    PlayerLost --> CheckPlayers{Plus de joueurs?}
    CheckPlayers -->|Oui| ShowLooser[looser]
    ShowLooser --> RestartGame[startSimonGame]
    CheckPlayers -->|Non| SortScores[sortScores2D]
    SortScores --> GameOver[gameOver]
    
    GameFinished --> SortScoresFinal[sortScores2D]
    SortScoresFinal --> CheckNewHS{Nouveau record?}
    CheckNewHS -->|Oui| SaveHS[highScore = scores0,1]
    CheckNewHS -->|Non| ShowVictory
    SaveHS --> ShowVictory[ledManager.victorySequence]
    ShowVictory --> Congratulate[congratulation]
    
    GameOver --> ShowGameOver[Animation Game Over]
    ShowGameOver --> UpdateLEDs
    
    RestartGame --> UpdateLEDs
    NextLevel --> UpdateLEDs
    Congratulate --> UpdateLEDs
    CheckComplete -->|Non| UpdateLEDs
    
    UpdateLEDs[ledManager.update] --> EndLoop[delay 5ms]
    EndLoop --> Loop
    
    style Start fill:#90EE90
    style Loop fill:#87CEEB
    style MenuState fill:#FFD700
    style GameState fill:#FF6B6B
    style GameOverState fill:#9370DB
    style StartGame fill:#FF8C00
    style HandleInput fill:#FF1493
    style GameWon fill:#00FF00
    style GameOver fill:#FF0000
    style GameFinished fill:#00CED1