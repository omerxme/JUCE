#!/bin/bash

echo "🚀 Building APT Suite - All Plugins"
echo "===================================="

# Array of plugins
plugins=("apt-chorus" "apt-drive" "apt-tilt" "apt-punch" "apt-wide")

for plugin in "${plugins[@]}"; do
    echo ""
    echo "📦 Building $plugin..."
    echo "--------------------"
    
    cd "$plugin"
    
    # Create build directory if it doesn't exist
    mkdir -p build
    cd build
    
    # Configure and build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release
    
    cd ../..
    
    echo "✅ $plugin complete!"
done

echo ""
echo "🎉 All plugins built successfully!"
echo ""
echo "Installed to:"
echo "  AU:  ~/Library/Audio/Plug-Ins/Components/"
echo "  VST3: ~/Library/Audio/Plug-Ins/VST3/"
echo ""
