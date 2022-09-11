MainComponent::MainComponent (SharedObjects&) :
    tabbedComponent (TabbedButtonBar::TabsAtTop)
{
    setOpaque (true);

    auto addTab = [&] (juce::StringRef name, Component* comp)
    {
        tabbedComponent.addTab (name, Colours::grey, comp, true);
    };

    addTab (TRANS ("Easing Demo"), new EaseListComponent());
    addTab (TRANS ("Image Demo"), new ImageDemo());

   #if USE_OPENGL
    // Need to call this later on - once JUCE, the GL content, and the OS decide it's cool to talk to each other.
    MessageManager::callAsync ([&]()
    {
        rendererConfigurator.configureWithOpenGLIfAvailable (*this);

        if (auto* context = rendererConfigurator.context.get())
            context->executeOnGLThread ([&] (OpenGLContext& c)
            {
                c.setSwapInterval (1);                  // Make sure vsync is active.
                rendererConfigurator.paintCallback();   // Need to force a repaint.
            }, false);
    });

    addTab (TRANS ("OpenGL Info"), new OpenGLDetailsDemo (rendererConfigurator));
   #endif

    addAndMakeVisible (tabbedComponent);
    setSize (1024, 768);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    tabbedComponent.setBounds (getLocalBounds ().reduced (4));
}
