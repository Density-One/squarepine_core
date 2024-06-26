#if SQUAREPINE_IS_DESKTOP

/** This is just a generic and simplistic tray icon component,
    instantiated on desktop systems only (which is the only place it's supported by JUCE).

    @see MainWindow, juce::SystemTrayIconComponent
*/
class TrayIconComponent final : public SystemTrayIconComponent
{
public:
    TrayIconComponent (ApplicationCommandManager& cm) :
        commandManager (cm)
    {
        setAccessible (false);

        const auto image = ImageCache::getFromMemory (BinaryData::SquarePine_Logo_png, BinaryData::SquarePine_Logo_pngSize);
        setIconImage (image, image);
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e) override
    {
        lastEventMods = e.mods;
    }

    void mouseUp (const MouseEvent&) override
    {
        auto& desktop = Desktop::getInstance();

        for (int i = desktop.getNumComponents(); --i >= 0;)
            desktop.getComponent (i)->toFront (true);

        // NB: by this point, the mouse buttons are... well, up/inactive.
        if (lastEventMods.isPopupMenu()
            && commandManager.getNumCommands() > 0)
        {
            PopupMenu pm;

            pm.addCommandItem (&commandManager, WorkstationIds::preferences);
            pm.addSeparator();
            pm.addCommandItem (&commandManager, StandardApplicationCommandIDs::quit);

            pm.showMenuAsync ({});
        }

        lastEventMods = {};
    }

private:
    //==============================================================================
    ApplicationCommandManager& commandManager;
    ModifierKeys lastEventMods;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrayIconComponent)
};

#endif // SQUAREPINE_IS_DESKTOP

//==============================================================================
class MainWindow final : public DocumentWindow
{
public:
    MainWindow (const String& name) :
        DocumentWindow (name, Colours::black, DocumentWindow::allButtons),
        customLookAndFeel (sharedObjects),
        mainComponent (sharedObjects)
    {
        googleAnalyticsReporter->startSession();

        setAccessible (false);
        setUsingNativeTitleBar (true);
        setOpaque (true);

        Desktop::getInstance().setDefaultLookAndFeel (&customLookAndFeel);

       #if SQUAREPINE_IS_DESKTOP
        trayIconComponent.reset (new TrayIconComponent (sharedObjects.commandManager));
       #endif

        setContentNonOwned (&mainComponent, true);

       #if SQUAREPINE_IS_MOBILE
        setFullScreen (true);
       #else
        setResizable (true, true);
        centreWithSize (getWidth(), getHeight());
       #endif

        setVisible (true);
    }

    ~MainWindow() override
    {
        Desktop::getInstance().setDefaultLookAndFeel (nullptr);
    }

    //==============================================================================
    /** @internal */
    void closeButtonPressed() override
    {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    //==============================================================================
    SharedObjects sharedObjects;
    SharedResourcePointer<sp::GoogleAnalyticsReporter> googleAnalyticsReporter;
    DemoLookAndFeel customLookAndFeel;

   #if SQUAREPINE_IS_DESKTOP
    std::unique_ptr<SystemTrayIconComponent> trayIconComponent;
   #endif

    MainComponent mainComponent;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

//==============================================================================
class SquarePineDemoApplication final : public sp::SimpleApplication
{
public:
    /** */
    SquarePineDemoApplication() = default;

    //==============================================================================
    /** @internal */
    const String getApplicationName() override      { return ProjectInfo::projectName; }
    /** @internal */
    const String getApplicationVersion() override   { return ProjectInfo::versionString; }

    //==============================================================================
    /** @internal */
    DocumentWindow* createWindow() const override
    {
        String windowName;
        windowName
            << ProjectInfo::companyName
            << " - " << ProjectInfo::projectName
            << " - v" << ProjectInfo::versionString;

       #if JUCE_DEBUG
        windowName << " [DEBUG]";
       #endif

        return new MainWindow (windowName);
    }

    String getLoggerFilePath() const override
    {
        return GlobalPaths::getLogFile().getParentDirectory().getFullPathName();
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquarePineDemoApplication)
};

//==============================================================================
START_JUCE_APPLICATION (SquarePineDemoApplication)
