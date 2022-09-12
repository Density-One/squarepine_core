#pragma once

#if USE_OPENGL

class OpenGLDetailsDemo final : public DemoBase,
                                public ListBoxModel
{
public:
    OpenGLDetailsDemo (SharedObjects& sharedObjs, HighPerformanceRendererConfigurator& rc) :
        DemoBase (sharedObjs, NEEDS_TRANS ("OpenGL Details Demo")),
        rendererConfigurator (rc)
    {
        SafePointer ptr (this);

        searchBox.onTextChange = [ptr]()
        {
            if (ptr != nullptr)
                ptr->refresh();
        };

        searchBox.setJustification (Justification::centredLeft);
        searchBox.setTextToShowWhenEmpty (TRANS ("(Type here to search)"), Colours::grey);

        addAndMakeVisible (searchBox);

        listbox.setModel (this);
        addAndMakeVisible (listbox);

        MessageManager::callAsync ([ptr]()
        {
            if (ptr == nullptr)
                return;

            if (auto* context = ptr->rendererConfigurator.context.get())
                context->executeOnGLThread ([ptr] (OpenGLContext&)
                {
                    // Can only be retrieved on the GL thread.
                    const auto details = getOpenGLDetails();

                    // Can only be set on the main message thread.
                    MessageManager::callAsync ([ptr, details]()
                    {
                        if (ptr != nullptr)
                        {
                            ptr->systemDetails = details;
                            ptr->refresh();
                        }
                    });
                }, false);
        });
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (4);

        searchBox.setBounds (b.removeFromTop (32));
        b.removeFromTop (4);
        listbox.setBounds (b);
    }

    int getNumRows() override { return displayDetails.size(); }

    static String getNameForRow (const StringPairArray& details, int row)
    {
        const auto key = details.getAllKeys()[row];
        const auto& value = details[key];
        return key + ": " + value;
    }

    String getNameForRow (int row) override
    {
        jassert (isPositiveAndBelow (row, getNumRows()));
        return getNameForRow (displayDetails, row);
    }

    void paintListBoxItem (int row, Graphics& g, int w, int h, bool) override
    {
        if (! isPositiveAndBelow (row, getNumRows()))
            return;

        const auto text = getNameForRow (row);
        g.setColour (findColour (ListBox::textColourId, true));
        g.drawFittedText (text, { w, h }, Justification::centredLeft, 1, 1.0f);
    }

private:
    HighPerformanceRendererConfigurator& rendererConfigurator;
    StringPairArray systemDetails, displayDetails;
    TextEditor searchBox;
    ListBox listbox;

    void refresh()
    {
        const auto searchText = searchBox.getText();

        displayDetails.clear();

        for (int i = 0; i < systemDetails.size(); ++i)
        {
            const auto key = systemDetails.getAllKeys()[i];
            const auto& value = systemDetails[key];

            if (key.containsIgnoreCase (searchText)
                || value.containsIgnoreCase (searchText))
            {
                displayDetails.set (key, value);
            }
        }

        listbox.updateContent();
    }

    static StringPairArray getOpenGLDetails()
    {
        using namespace juce::gl;

        StringPairArray result;

        auto addStringResult = [&] (const String& name, const String& details)
        {
            jassert (! result.containsKey (name));

            if (details.isNotEmpty())
                result.set (name, details);
        };

        auto addResult = [&] (const String& name, GLenum value)
        {
            addStringResult (name, sp::getGLString (value).trim());
        };

        #undef SP_ADD_LOG_ITEM
        #define SP_ADD_LOG_ITEM(x) \
            addResult (#x, x);

        SP_ADD_LOG_ITEM (GL_VENDOR)
        SP_ADD_LOG_ITEM (GL_RENDERER)
        SP_ADD_LOG_ITEM (GL_VERSION)
        SP_ADD_LOG_ITEM (GL_SHADING_LANGUAGE_VERSION)
        SP_ADD_LOG_ITEM (GL_INFO_LOG_LENGTH)

        GLint major = 0, minor = 0, numExtensions = 0;
        glGetIntegerv (GL_MAJOR_VERSION, &major);
        glGetIntegerv (GL_MINOR_VERSION, &minor);
        glGetIntegerv (GL_NUM_EXTENSIONS, &numExtensions);

        addStringResult ("GL_MAJOR_VERSION", String (major));
        addStringResult ("GL_MINOR_VERSION", String (minor));
        addStringResult ("GL_NUM_EXTENSIONS", String (numExtensions));

        {
            auto extensionsFromGL = sp::getGLString (GL_EXTENSIONS).trim();
            for (GLuint i = 0; i < (GLuint) numExtensions; ++i)
                extensionsFromGL << sp::getGLString (GL_EXTENSIONS, i) << " ";

            auto ext = StringArray::fromTokens (extensionsFromGL, " ", "");
            ext.trim();
            ext.removeEmptyStrings();
            ext.removeDuplicates (true);
            ext.sort (true);

            for (int i = 0; i < ext.size(); ++i)
                addStringResult ("Extension " + String (i + 1), ext.getReference (i));
        }

        SP_ADD_LOG_ITEM (GL_MAX_3D_TEXTURE_SIZE_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_3D_TEXTURE_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_4D_TEXTURE_SIZE_SGIS)
        SP_ADD_LOG_ITEM (GL_MAX_ACTIVE_LIGHTS_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_ARRAY_TEXTURE_LAYERS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_ARRAY_TEXTURE_LAYERS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_ARRAY_TEXTURE_LAYERS)
        SP_ADD_LOG_ITEM (GL_MAX_ASYNC_DRAW_PIXELS_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_ASYNC_HISTOGRAM_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_ASYNC_READ_PIXELS_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_ASYNC_TEX_IMAGE_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS)
        SP_ADD_LOG_ITEM (GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_ATTRIB_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_BINDABLE_UNIFORM_SIZE_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_CLIENT_ATTRIB_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_CLIENT_ATTRIB_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_CLIP_DISTANCES)
        SP_ADD_LOG_ITEM (GL_MAX_CLIP_PLANES)
        SP_ADD_LOG_ITEM (GL_MAX_CLIPMAP_DEPTH_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_COARSE_FRAGMENT_SAMPLES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_ATTACHMENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_ATTACHMENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_FRAMEBUFFER_STORAGE_SAMPLES_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_MATRIX_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_COLOR_TEXTURE_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_DIMENSIONS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_MESH_UNIFORM_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_TASK_UNIFORM_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_FIXED_GROUP_INVOCATIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_FIXED_GROUP_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_SHARED_MEMORY_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_WORK_GROUP_COUNT)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS)
        SP_ADD_LOG_ITEM (GL_MAX_COMPUTE_WORK_GROUP_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_CONVOLUTION_HEIGHT_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_CONVOLUTION_HEIGHT)
        SP_ADD_LOG_ITEM (GL_MAX_CONVOLUTION_WIDTH_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_CONVOLUTION_WIDTH)
        SP_ADD_LOG_ITEM (GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_CUBE_MAP_TEXTURE_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_CULL_DISTANCES)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_GROUP_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_LOGGED_MESSAGES_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_LOGGED_MESSAGES_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_LOGGED_MESSAGES_KHR)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_LOGGED_MESSAGES)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_MESSAGE_LENGTH_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_MESSAGE_LENGTH_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_MESSAGE_LENGTH_KHR)
        SP_ADD_LOG_ITEM (GL_MAX_DEBUG_MESSAGE_LENGTH)
        SP_ADD_LOG_ITEM (GL_MAX_DEEP_3D_TEXTURE_DEPTH_NV)
        SP_ADD_LOG_ITEM (GL_MAX_DEEP_3D_TEXTURE_WIDTH_HEIGHT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_DEFORMATION_ORDER_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_DEPTH_STENCIL_FRAMEBUFFER_SAMPLES_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_DEPTH_TEXTURE_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_DETACHED_BUFFERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_DETACHED_TEXTURES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_DRAW_BUFFERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_DRAW_BUFFERS_ATI)
        SP_ADD_LOG_ITEM (GL_MAX_DRAW_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_DRAW_MESH_TASKS_COUNT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_DRAW_MESH_TASKS_COUNT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_DUAL_SOURCE_DRAW_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_ELEMENT_INDEX)
        SP_ADD_LOG_ITEM (GL_MAX_ELEMENTS_INDICES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_ELEMENTS_INDICES)
        SP_ADD_LOG_ITEM (GL_MAX_ELEMENTS_VERTICES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_ELEMENTS_VERTICES)
        SP_ADD_LOG_ITEM (GL_MAX_EVAL_ORDER)
        SP_ADD_LOG_ITEM (GL_MAX_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_FOG_FUNC_POINTS_SGIS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_BINDABLE_UNIFORMS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_INPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_NV)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_INTERPOLATION_OFFSET)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_LIGHTS_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_PROGRAM_LOCAL_PARAMETERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAGMENT_UNIFORM_VECTORS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAMEBUFFER_HEIGHT)
        SP_ADD_LOG_ITEM (GL_MAX_FRAMEBUFFER_LAYERS)
        SP_ADD_LOG_ITEM (GL_MAX_FRAMEBUFFER_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_FRAMEBUFFER_WIDTH)
        SP_ADD_LOG_ITEM (GL_MAX_FRAMEZOOM_FACTOR_SGIX)
        SP_ADD_LOG_ITEM (GL_MAX_GENERAL_COMBINERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_BINDABLE_UNIFORMS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_INPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_OUTPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_OUTPUT_VERTICES)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_PROGRAM_INVOCATIONS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_SHADER_INVOCATIONS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_VARYING_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_HEIGHT)
        SP_ADD_LOG_ITEM (GL_MAX_IMAGE_SAMPLES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_IMAGE_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_IMAGE_UNITS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_INTEGER_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_INTEGER_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_LABEL_LENGTH_KHR)
        SP_ADD_LOG_ITEM (GL_MAX_LABEL_LENGTH_KHR)
        SP_ADD_LOG_ITEM (GL_MAX_LABEL_LENGTH)
        SP_ADD_LOG_ITEM (GL_MAX_LAYERS)
        SP_ADD_LOG_ITEM (GL_MAX_LGPU_GPUS_NVX)
        SP_ADD_LOG_ITEM (GL_MAX_LIGHTS)
        SP_ADD_LOG_ITEM (GL_MAX_LIST_NESTING)
        SP_ADD_LOG_ITEM (GL_MAX_MAP_TESSELLATION_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_ATOMIC_COUNTER_BUFFERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_ATOMIC_COUNTERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_IMAGE_UNIFORMS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_OUTPUT_PRIMITIVES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_OUTPUT_VERTICES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_SHADER_STORAGE_BLOCKS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_TEXTURE_IMAGE_UNITS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_TOTAL_MEMORY_SIZE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_UNIFORM_BLOCKS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_UNIFORM_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_VIEWS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_WORK_GROUP_INVOCATIONS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MESH_WORK_GROUP_SIZE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_MODELVIEW_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_NAME_LENGTH)
        SP_ADD_LOG_ITEM (GL_MAX_NAME_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_NUM_ACTIVE_VARIABLES)
        SP_ADD_LOG_ITEM (GL_MAX_NUM_COMPATIBLE_SUBROUTINES)
        SP_ADD_LOG_ITEM (GL_MAX_OPTIMIZED_VERTEX_SHADER_INSTRUCTIONS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_OPTIMIZED_VERTEX_SHADER_INVARIANTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCAL_CONSTANTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_OPTIMIZED_VERTEX_SHADER_LOCALS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_OPTIMIZED_VERTEX_SHADER_VARIANTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_PALETTE_MATRICES_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PATCH_VERTICES)
        SP_ADD_LOG_ITEM (GL_MAX_PIXEL_MAP_TABLE)
        SP_ADD_LOG_ITEM (GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_ALU_INSTRUCTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_ATTRIB_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_ATTRIBS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_CALL_DEPTH_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_ENV_PARAMETERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_EXEC_INSTRUCTIONS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_GENERIC_ATTRIBS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_GENERIC_RESULTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_IF_DEPTH_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_INSTRUCTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_LOOP_COUNT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_LOOP_DEPTH_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_MATRICES_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_ALU_INSTRUCTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_TEX_INDIRECTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_NATIVE_TEX_INSTRUCTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_OUTPUT_VERTICES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_PARAMETER_BUFFER_BINDINGS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_PARAMETER_BUFFER_SIZE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_PARAMETERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_PATCH_ATTRIBS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_RESULT_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_SUBROUTINE_NUM_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_SUBROUTINE_PARAMETERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEMPORARIES_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEX_INDIRECTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEX_INSTRUCTIONS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXEL_OFFSET_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXEL_OFFSET_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXEL_OFFSET)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXTURE_GATHER_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET)
        SP_ADD_LOG_ITEM (GL_MAX_PROGRAM_TOTAL_OUTPUT_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_PROJECTION_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_RASTER_SAMPLES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_RATIONAL_EVAL_ORDER_NV)
        SP_ADD_LOG_ITEM (GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_RECTANGLE_TEXTURE_SIZE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_RECTANGLE_TEXTURE_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_RENDERBUFFER_SIZE_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_RENDERBUFFER_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_SAMPLE_MASK_WORDS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_SAMPLE_MASK_WORDS)
        SP_ADD_LOG_ITEM (GL_MAX_SAMPLES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_SAMPLES)
        SP_ADD_LOG_ITEM (GL_MAX_SERVER_WAIT_TIMEOUT)
        SP_ADD_LOG_ITEM (GL_MAX_SHADER_BUFFER_ADDRESS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_SHADER_COMPILER_THREADS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_SHADER_COMPILER_THREADS_KHR)
        SP_ADD_LOG_ITEM (GL_MAX_SHADER_STORAGE_BLOCK_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS)
        SP_ADD_LOG_ITEM (GL_MAX_SHININESS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_SPARSE_3D_TEXTURE_SIZE_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS)
        SP_ADD_LOG_ITEM (GL_MAX_SPARSE_TEXTURE_SIZE_AMD)
        SP_ADD_LOG_ITEM (GL_MAX_SPARSE_TEXTURE_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_SPOT_EXPONENT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_SUBPIXEL_PRECISION_BIAS_BITS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS)
        SP_ADD_LOG_ITEM (GL_MAX_SUBROUTINES)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_ATOMIC_COUNTER_BUFFERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_ATOMIC_COUNTERS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_IMAGE_UNIFORMS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_OUTPUT_COUNT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_SHADER_STORAGE_BLOCKS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_TEXTURE_IMAGE_UNITS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_TOTAL_MEMORY_SIZE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_UNIFORM_BLOCKS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_UNIFORM_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_WORK_GROUP_INVOCATIONS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TASK_WORK_GROUP_SIZE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_INPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_GEN_LEVEL)
        SP_ADD_LOG_ITEM (GL_MAX_TESS_PATCH_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_BUFFER_SIZE_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_BUFFER_SIZE_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_BUFFER_SIZE_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_BUFFER_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_COORDS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_COORDS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_COORDS)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_IMAGE_UNITS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_IMAGE_UNITS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_LOD_BIAS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_LOD_BIAS)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_MAX_ANISOTROPY)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_STACK_DEPTH)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_UNITS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_TEXTURE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_TIMELINE_SEMAPHORE_VALUE_DIFFERENCE_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TRACK_MATRICES_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TRACK_MATRIX_STACK_DEPTH_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_NV)
        SP_ADD_LOG_ITEM (GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_UNIFORM_BLOCK_SIZE)
        SP_ADD_LOG_ITEM (GL_MAX_UNIFORM_BUFFER_BINDINGS)
        SP_ADD_LOG_ITEM (GL_MAX_UNIFORM_LOCATIONS)
        SP_ADD_LOG_ITEM (GL_MAX_VARYING_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VARYING_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_VARYING_FLOATS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_VARYING_FLOATS)
        SP_ADD_LOG_ITEM (GL_MAX_VARYING_VECTORS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATOMIC_COUNTERS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATTRIB_BINDINGS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATTRIB_STRIDE)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATTRIBS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_ATTRIBS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_HINT_PGI)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_IMAGE_UNIFORMS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_OUTPUT_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_SHADER_INSTRUCTIONS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_SHADER_INVARIANTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_SHADER_LOCAL_CONSTANTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_SHADER_LOCALS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_SHADER_VARIANTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_STREAMS_ATI)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_STREAMS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_UNIFORM_BLOCKS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_UNIFORM_COMPONENTS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_UNIFORM_VECTORS)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_UNITS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_VARYING_COMPONENTS_ARB)
        SP_ADD_LOG_ITEM (GL_MAX_VERTEX_VARYING_COMPONENTS_EXT)
        SP_ADD_LOG_ITEM (GL_MAX_VIEWPORT_DIMS)
        SP_ADD_LOG_ITEM (GL_MAX_VIEWPORTS)
        SP_ADD_LOG_ITEM (GL_MAX_VIEWS_OVR)
        SP_ADD_LOG_ITEM (GL_MAX_WIDTH)
        SP_ADD_LOG_ITEM (GL_MAX_WINDOW_RECTANGLES_EXT)
        SP_ADD_LOG_ITEM (GL_MAX)

        SP_ADD_LOG_ITEM (GL_MIN_EXT)
        SP_ADD_LOG_ITEM (GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_NV)
        SP_ADD_LOG_ITEM (GL_MIN_FRAGMENT_INTERPOLATION_OFFSET)
        SP_ADD_LOG_ITEM (GL_MIN_PROGRAM_TEXEL_OFFSET_EXT)
        SP_ADD_LOG_ITEM (GL_MIN_PROGRAM_TEXEL_OFFSET_NV)
        SP_ADD_LOG_ITEM (GL_MIN_PROGRAM_TEXEL_OFFSET)
        SP_ADD_LOG_ITEM (GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_ARB)
        SP_ADD_LOG_ITEM (GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET_NV)
        SP_ADD_LOG_ITEM (GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET)
        SP_ADD_LOG_ITEM (GL_MIN)

        #undef SP_ADD_LOG_ITEM

        result.minimiseStorageOverheads();
        return result;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDetailsDemo)
};

#endif // USE_OPENGL
