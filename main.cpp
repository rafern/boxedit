/*
    A quick and dirty pointybox file format editor.
    Meant to be used for developing games with SFTE.
    pointybox files hold data for:
        - AABBs of a tileset.
        - Tile corners for light rendering.
*/

#include "pointybox.hpp"
#include <iostream>
#include <math.h>

void renderPointMode(sf::VertexArray* pointyboxVA, std::vector < sf::Vector2i >* pointVec, sf::Color pointColor, float& zoom) {
    for(size_t i = 0; i < pointVec->size(); ++i) {
        sf::Vector2i pos = pointVec->at(i);
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.x * zoom, pos.y * zoom), pointColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f((pos.x + 1) * zoom, pos.y * zoom), pointColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f((pos.x + 1) * zoom, (pos.y + 1) * zoom), pointColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.x * zoom, (pos.y + 1) * zoom), pointColor));
    }
}

void renderAABBMode(sf::VertexArray* pointyboxVA, std::vector < sf::IntRect >* aabbVec, sf::Color aabbColor, float& zoom) {
    for(size_t i = 0; i < aabbVec->size(); ++i) {
        sf::IntRect pos = aabbVec->at(i);
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.left * zoom, pos.top * zoom), aabbColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f((pos.left + pos.width) * zoom, pos.top * zoom), aabbColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f((pos.left + pos.width) * zoom, (pos.top + pos.height) * zoom), aabbColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.left * zoom, (pos.top + pos.height) * zoom), aabbColor));
    }
}

void renderEdgeMode(sf::VertexArray* pointyboxVA, std::vector < sf::IntRect >* edgeVec, sf::Color edgeColor, float& zoom, char lineThickness) {
    for(size_t i = 0; i < edgeVec->size(); ++i) {
        sf::IntRect pos = edgeVec->at(i);
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.left * zoom - lineThickness - 1, pos.top * zoom - lineThickness - 1), edgeColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.width * zoom + lineThickness, pos.top * zoom - lineThickness - 1), edgeColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.width * zoom + lineThickness, pos.height * zoom + lineThickness), edgeColor));
        pointyboxVA->append(sf::Vertex(sf::Vector2f(pos.left * zoom - lineThickness - 1, pos.height * zoom + lineThickness), edgeColor));
    }
}

sf::IntRect getAABBFromPoints(sf::Vector2i p1, sf::Vector2i p2) {
    sf::Vector2i tl,
                 br;
    if(p1.x > p2.x) {
        tl.x = p2.x;
        br.x = p1.x;
    }
    else {
        tl.x = p1.x;
        br.x = p2.x;
    }
    if(p1.y > p2.y) {
        tl.y = p2.y;
        br.y = p1.y;
    }
    else {
        tl.y = p1.y;
        br.y = p2.y;
    }
    return sf::IntRect(tl.x, tl.y, br.x - tl.x + 1, br.y - tl.y + 1);
}

sf::IntRect getEdgeFromPoints(sf::Vector2i p1, sf::Vector2i p2) {
    sf::Vector2i tl,
                 br;
    if(p1.x > p2.x) {
        tl.x = p2.x;
        br.x = p1.x;
    }
    else {
        tl.x = p1.x;
        br.x = p2.x;
    }
    if(p1.y > p2.y) {
        tl.y = p2.y;
        br.y = p1.y;
    }
    else {
        tl.y = p1.y;
        br.y = p2.y;
    }
    return sf::IntRect(tl.x, tl.y, br.x, br.y);
}

sf::Vector2i snapEdge(sf::Vector2i pos, sf::Vector2i snapTo) {
    sf::Vector2i dif(snapTo.x - pos.x, snapTo.y - pos.y);
    if(dif.x < 0)
        dif.x *= -1;
    if(dif.y < 0)
        dif.y *= -1;
    if(dif.x <= dif.y) // Prefer x over y when x == y. Snap to x
        return sf::Vector2i(snapTo.x, pos.y);
    else // Snap to y
        return sf::Vector2i(pos.x, snapTo.y);
}

int main(int argc, char* argv[]) {
    try {
        if((argc == 2) || (argc == 3)) {
            pb::PointyboxLoader ploader(argv[1]);
            sf::Vector2u resolution(8, 8);        // Current resolution of whole file
            // Other PB data
            pb::AABBVectorRaw aabbVec;
            pb::PointVectorRaw pointVec;
            pb::EdgeVectorRaw edgeVec;
            
            if (ploader.load(&resolution, &aabbVec, &pointVec, &edgeVec))
                std::cout << "Loaded pointybox file " << argv[1] << std::endl;
            else {
                std::cerr << "Error: " << argv[1] << " is not a valid pointybox file! By proceeding, this file will be replaced! Proceed? Anything which isn't exactly YES will exit the program." << std::endl;
                std::string input;
                std::cin >> input;
                if (input != "YES")
                    return EXIT_FAILURE;
                
                resolution = sf::Vector2u(8, 8);
                aabbVec.clear();
                pointVec.clear();
                edgeVec.clear();
            }
            if(aabbVec.empty())
                aabbVec.push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));
            if(pointVec.empty())
                pointVec.push_back(std::vector < std::vector < sf::Vector2i > >(47, std::vector < sf::Vector2i >()));
            if(edgeVec.empty())
                edgeVec.push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));

            // Config and other stuff
            bool drag = false,                              // Dragging?
                 textureDrag = false,                       // Dragging background image?
                 selAABB = false,                           // Selecting second aabb point?
                 selEdge = false,                           // Selecting second edge point?
                 showGrid = true,                           // Enable grid? (independent of zoom)
                 renderTex = (argc == 3),                   // Render background image?
                 fullscreen = false,                        // Is the window currently fullscreen?
                 redraw = true;                             // Redraw screen? Reduces GPU load when static.
            float zoom = 32.0f;                             // Zoom level
            unsigned char bitmask = 0,                      // Selected bitmask
                          mode = 0,                         // Edit mode. 0 = aabb, 1 = point, 2 = edge
                          selColour = 0,                    // Current bg colour
                          edgeThickness = 3,                // Edge line thickness for rendering
                          thisEdgeThickness = edgeThickness;// Edge line thickness's final value. = edgeThickness * (zoom / 32)
            size_t id = 0;                                  // Selected tile ID
            sf::Vector2i mousePos,                          // Current mouse position
                         selectedTilePos,                   // Current selected tile position (calculated from mousePos, camPos and zoom)
                         firstPos,                          // Selected position for first aabb or edge point
                         texSize;                           // Current texture size "offset"
            sf::Vector2f camPos(0.0f, 0.0f),                // Current camera position
                         texCamPos(0.0f, 0.0f);             // Current background image position
                                                            // Colours used for background:
            sf::Color colours[3] = {sf::Color::Black,           // Black
                                    sf::Color(127, 127, 127),   // Gray
                                    sf::Color(127, 0  , 127)};  // Purple
                                                            // Strings for help and stats:
            std::string help = "[Q] Quit; [M] Mode; [Up/Down] Bitmask; [Left/Right] ID; [LMB/RMB] Add/remove; [IJKL] Resolution\nCamera: [F] Fullscreen; [C] BG colour; [WMB drag] Move camera; [T drag] Move texture; [WASD] Texture size; [G] Grid\n",
                        modeInfo[3] = {"AABBs",
                                       "Points",
                                       "Edges"},
                        bitmaskInfo[47] = {"Island (not surrounded by any connective tile)",
                                           "Up",
                                           "Left",
                                           "Left & up",
                                           "Left, up & top-left",
                                           "Right",
                                           "Right & up",
                                           "Right, up & top-right",
                                           "Left & right",
                                           "Left, right & up",
                                           "Left, right, up & top-left",
                                           "Left, right, up & top-right",
                                           "Left, right, up, top-left & top-right",
                                           "Down",
                                           "Up & down",
                                           "Left & down",
                                           "Left, up & down",
                                           "Left, up, down & top-left",
                                           "Right & down",
                                           "Right, up & down",
                                           "Right, up, down & top-right",
                                           "Left, right & down",
                                           "Left, right, up & down",
                                           "Left, right, up, down & top-left",
                                           "Left, right, up, down & top-right",
                                           "Left, right, up, down, top-left & top-right",
                                           "Left, down & bottom-left",
                                           "Left, up, down & bottom-left",
                                           "Left, up, down, top-left & bottom-left",
                                           "Left, right, down & bottom-left",
                                           "Left, right, up, down & bottom-left",
                                           "Left, right, up, down, top-left & bottom-left",
                                           "Left, right, up, down, top-right & bottom-left",
                                           "Left, right, up, down, top-left, top-right & bottom-left",
                                           "Right, down & bottom-right",
                                           "Right, up, down & bottom-right",
                                           "Right, up, down, top-right & bottom-right",
                                           "Left, right, down & bottom-right",
                                           "Left, right, up, down & bottom-right",
                                           "Left, right, up, down, top-left & bottom-right",
                                           "Left, right, up, down, top-right & bottom-right",
                                           "Left, right, up, down, top-left, top-right & bottom-right",
                                           "Left, right, down, bottom-left & bottom-right",
                                           "Left, right, up, down, bottom-left & bottom-right",
                                           "Left, right, up, down, top-left, bottom-left & bottom-right",
                                           "Left, right, up, down, top-right, bottom-left & bottom-right",
                                           "Fully surrounded (or without bitmask)"};

            // Graphics objects
            sf::VertexArray gridVA(sf::Lines),
                            pointyboxVA(sf::Quads);
            sf::Font gnuUnifont;
            gnuUnifont.loadFromFile("unifont-9.0.06.ttf");
            sf::Text infoText("", gnuUnifont, 12);
            sf::Texture texture;
            sf::RectangleShape texRect;

            if(argc == 3) {
                if(!texture.loadFromFile(argv[2])) {
                    std::cerr << "Error: " << argv[2] << " is not a valid texture file!" << std::endl;
                    return 0;
                }
                texRect.setSize(sf::Vector2f(texture.getSize().x * zoom, texture.getSize().y * zoom));
                texRect.setTexture(&texture);
            }

            sf::RenderWindow window(sf::VideoMode(800, 640), "BoxEdit - " + std::string(argv[1]));
            sf::Vector2u winSize(window.getSize());
            window.setFramerateLimit(60);
            while(window.isOpen()) {
                sf::Vector2i lastMousePos = mousePos;
                mousePos = sf::Mouse::getPosition(window);
                sf::Vector2i displacement = sf::Vector2i(mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y);
                if((displacement.x != 0) || (displacement.y != 0)) {
                    if(drag) {
                        sf::View view(window.getView());
                        camPos.x -= displacement.x;
                        camPos.y -= displacement.y;
                        view.setCenter(camPos.x + (winSize.x * 0.5f), camPos.y + (winSize.y * 0.5f));
                        window.setView(view);
                    }
                    else if(textureDrag) {
                        texCamPos.x += displacement.x / zoom;
                        texCamPos.y += displacement.y / zoom;
                        texRect.setPosition(sf::Vector2f(floor(texCamPos.x) * zoom, floor(texCamPos.y) * zoom));
                    }
                    redraw = true;
                }
                sf::Vector2i lastSelectedTilePos = selectedTilePos;
                selectedTilePos = sf::Vector2i(floor((mousePos.x + camPos.x) / zoom), floor((mousePos.y + camPos.y) / zoom));
                if(lastSelectedTilePos != selectedTilePos)
                    redraw = true;

                sf::Event event;
                while (window.pollEvent(event)) {
                    switch(event.type) {
                    case sf::Event::Closed:
                        window.close();
                        break;
                    case sf::Event::Resized:
                        {
                        redraw = true;
                        winSize = window.getSize();
                        sf::View view;
                        view.setSize(winSize.x, winSize.y);
                        view.setCenter(camPos.x + (winSize.x * 0.5f), camPos.y + (winSize.y * 0.5f));
                        window.setView(view);
                        }
                        break;
                    case sf::Event::KeyPressed:
                        redraw = true;
                        switch(event.key.code) {
                        case sf::Keyboard::Q:
                            window.close();
                            break;
                        case sf::Keyboard::F:
                            {
                            fullscreen = !fullscreen;
                            if(fullscreen)
                                window.create(sf::VideoMode::getFullscreenModes()[0], "BoxEdit - " + std::string(argv[1]));
                            else
                                window.create(sf::VideoMode(800, 640), "BoxEdit - " + std::string(argv[1]));
                            winSize = window.getSize();
                            sf::View view(window.getView());
                            view.setCenter(camPos.x + (winSize.x * 0.5f), camPos.y + (winSize.y * 0.5f));
                            window.setView(view);
                            }
                            break;
                        case sf::Keyboard::M:
                            selAABB = false;
                            ++mode;
                            if(mode > 2)
                                mode = 0;
                            break;
                        case sf::Keyboard::C:
                            ++selColour;
                            if(selColour > 2)
                                selColour = 0;
                            break;
                        case sf::Keyboard::T:
                            textureDrag = true;
                            break;
                        case sf::Keyboard::G:
                            showGrid = !showGrid;
                            break;
                        case sf::Keyboard::W:
                            texSize.y -= 1;
                            texRect.setSize(sf::Vector2f((texture.getSize().x + texSize.x) * zoom, (texture.getSize().y + texSize.y) * zoom));
                            break;
                        case sf::Keyboard::S:
                            texSize.y += 1;
                            texRect.setSize(sf::Vector2f((texture.getSize().x + texSize.x) * zoom, (texture.getSize().y + texSize.y) * zoom));
                            break;
                        case sf::Keyboard::A:
                            texSize.x -= 1;
                            texRect.setSize(sf::Vector2f((texture.getSize().x + texSize.x) * zoom, (texture.getSize().y + texSize.y) * zoom));
                            break;
                        case sf::Keyboard::D:
                            texSize.x += 1;
                            texRect.setSize(sf::Vector2f((texture.getSize().x + texSize.x) * zoom, (texture.getSize().y + texSize.y) * zoom));
                            break;
                        case sf::Keyboard::I:
                            if(resolution.y > 1)
                                resolution.y -= 1;
                            break;
                        case sf::Keyboard::K:
                            resolution.y += 1;
                            break;
                        case sf::Keyboard::J:
                            if(resolution.x > 1)
                                resolution.x -= 1;
                            break;
                        case sf::Keyboard::L:
                            resolution.x += 1;
                            break;
                        case sf::Keyboard::Up:
                            if(bitmask > 0)
                                --bitmask;
                            break;
                        case sf::Keyboard::Down:
                            if(bitmask < 46)
                                ++bitmask;
                            break;
                        case sf::Keyboard::Left:
                            if(id > 0)
                                --id;
                            break;
                        case sf::Keyboard::Right:
                            ++id;
                            if(id >= aabbVec.size()) {
                                aabbVec.push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));
                                pointVec.push_back(std::vector < std::vector < sf::Vector2i > >(47, std::vector < sf::Vector2i >()));
                                edgeVec.push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));
                            }
                            break;
                        }
                        break;
                    case sf::Event::KeyReleased:
                        switch(event.key.code) {
                        case sf::Keyboard::T:
                            textureDrag = false;
                            break;
                        }
                        break;
                    case sf::Event::MouseButtonPressed:
                        redraw = true;
                        switch(event.mouseButton.button) {
                        case sf::Mouse::Left:
                            switch(mode) {
                            case 0:
                                if(selAABB) {
                                    selAABB = false;
                                    sf::IntRect val(getAABBFromPoints(selectedTilePos, firstPos));
                                    if(std::find(aabbVec[id][bitmask].begin(), aabbVec[id][bitmask].end(), val) == aabbVec[id][bitmask].end())
                                        aabbVec[id][bitmask].push_back(val);
                                }
                                else {
                                    selAABB = true;
                                    firstPos = selectedTilePos;
                                }
                                break;
                            case 1:
                                if(std::find(pointVec[id][bitmask].begin(), pointVec[id][bitmask].end(), selectedTilePos) == pointVec[id][bitmask].end())
                                    pointVec[id][bitmask].push_back(selectedTilePos);
                                break;
                            case 2:
                                if(selEdge) {
                                    sf::IntRect val(getEdgeFromPoints(snapEdge(selectedTilePos, firstPos), firstPos));
                                    if((val.left == val.width) && (val.top == val.height))
                                        break;
                                    selEdge = false;
                                    if(std::find(edgeVec[id][bitmask].begin(), edgeVec[id][bitmask].end(), val) == edgeVec[id][bitmask].end())
                                        edgeVec[id][bitmask].push_back(val);
                                }
                                else {
                                    selEdge = true;
                                    firstPos = selectedTilePos;
                                }
                                break;
                            }
                            break;
                        case sf::Mouse::Right:
                            switch(mode) {
                            case 0:
                                if(selAABB)
                                    selAABB = false;
                                else {
                                    size_t size = aabbVec[id][bitmask].size(),
                                           smallI = 0,
                                           smallS = 0;
                                    bool found = false;

                                    for(size_t i = 0; i < size; ++i) {
                                        if ((selectedTilePos.x >= aabbVec[id][bitmask][i].left) &&
                                            (selectedTilePos.x <= aabbVec[id][bitmask][i].left + aabbVec[id][bitmask][i].width) &&
                                            (selectedTilePos.y >= aabbVec[id][bitmask][i].top) &&
                                            (selectedTilePos.y <= aabbVec[id][bitmask][i].top + aabbVec[id][bitmask][i].height)) {
                                            size_t thisSize = (aabbVec[id][bitmask][i].width + 1) * (aabbVec[id][bitmask][i].height + 1);
                                            if (!found || (thisSize <= smallS)) { // <= instead of < so it prefers more recent boxes
                                                smallI = i;
                                                smallS = thisSize;
                                                found = true;
                                            }
                                        }
                                    }

                                    if(found)
                                        aabbVec[id][bitmask].erase(aabbVec[id][bitmask].begin() + smallI);
                                }
                                break;
                            case 1:
                                {
                                std::vector < sf::Vector2i >::iterator foundit(std::find(pointVec[id][bitmask].begin(), pointVec[id][bitmask].end(), selectedTilePos));
                                if(foundit != pointVec[id][bitmask].end())
                                    pointVec[id][bitmask].erase(foundit);
                                }
                                break;
                            case 2:
                                if(selEdge)
                                    selEdge = false;
                                else {
                                    size_t size = edgeVec[id][bitmask].size(),
                                           smallI = 0,
                                           smallS = 0;
                                    bool found = false;

                                    for(size_t i = 0; i < size; ++i) {
                                        if ((selectedTilePos.x >= edgeVec[id][bitmask][i].left) &&
                                            (selectedTilePos.x <= edgeVec[id][bitmask][i].width) &&
                                            (selectedTilePos.y >= edgeVec[id][bitmask][i].top) &&
                                            (selectedTilePos.y <= edgeVec[id][bitmask][i].height)) {
                                            size_t thisSize = (edgeVec[id][bitmask][i].width - edgeVec[id][bitmask][i].left + 1) * (edgeVec[id][bitmask][i].height - edgeVec[id][bitmask][i].top + 1);
                                            if (!found || (thisSize <= smallS)) { // <= instead of < so it prefers more recent edges
                                                smallI = i;
                                                smallS = thisSize;
                                                found = true;
                                            }
                                        }
                                    }

                                    if(found)
                                        edgeVec[id][bitmask].erase(edgeVec[id][bitmask].begin() + smallI);
                                }
                                break;
                            }
                            break;
                        case sf::Mouse::Middle:
                            drag = true;
                        }
                        break;
                    case sf::Event::MouseButtonReleased:
                        switch(event.mouseButton.button) {
                        case sf::Mouse::Middle:
                            drag = false;
                        }
                        break;
                    case sf::Event::MouseWheelScrolled:
                        redraw = true;
                        if(event.mouseWheelScroll.delta < 0) {
                            if(zoom > 4)
                                --zoom;
                        }
                        else {
                            if(zoom < 32)
                                ++zoom;
                        }
                        texRect.setSize(sf::Vector2f((texture.getSize().x + texSize.x) * zoom, (texture.getSize().y + texSize.y) * zoom));
                        texRect.setPosition(sf::Vector2f(floor(texCamPos.x) * zoom, floor(texCamPos.y) * zoom));
                        break;
                    }
                }

                if(redraw) {
                    redraw = false;
                    infoText.setString(help + "Mode: " + modeInfo[mode] + "\nSelected pixel: " + std::to_string(selectedTilePos.x) + ", " + std::to_string(selectedTilePos.y) + "\nBitmask: " + bitmaskInfo[bitmask] + " tile\nCurrent ID:" + std::to_string(id) + "\nCurrent texture size \"offset\":" + std::to_string(texSize.x) + "," + std::to_string(texSize.y) + "\nResolution:" + std::to_string(resolution.x) + "," + std::to_string(resolution.y));
                    infoText.setPosition(camPos);

                    window.clear(colours[selColour]);
                    gridVA.clear();
                    pointyboxVA.clear();

                    // Render bg texture
                    if(renderTex)
                        window.draw(texRect);

                    // Render grid
                    long long camXTL = floor(camPos.x / zoom),
                              camYTL = floor(camPos.y / zoom),
                              camXBR = ceil((camPos.x + winSize.x) / zoom),
                              camYBR = ceil((camPos.y + winSize.y) / zoom);
                    if((zoom >= 8) && showGrid) {
                        for(long long x = camXTL; x <= camXBR; ++x) {
                            sf::Color thisColor((x == 0) ? sf::Color::Red : sf::Color(127, 127, 127));
                            gridVA.append(sf::Vertex(sf::Vector2f(x * zoom, camYTL * zoom), thisColor));
                            gridVA.append(sf::Vertex(sf::Vector2f(x * zoom, camYBR * zoom), thisColor));
                        }

                        for(long long y = camYTL; y <= camYBR; ++y) {
                            sf::Color thisColor((y == 0) ? sf::Color::Red : sf::Color(127, 127, 127));
                            gridVA.append(sf::Vertex(sf::Vector2f(camXTL * zoom, y * zoom), thisColor));
                            gridVA.append(sf::Vertex(sf::Vector2f(camXBR * zoom, y * zoom), thisColor));
                        }
                    }
                    else {
                        gridVA.append(sf::Vertex(sf::Vector2f(0, camYTL * zoom), sf::Color::Red));
                        gridVA.append(sf::Vertex(sf::Vector2f(0, camYBR * zoom), sf::Color::Red));
                        gridVA.append(sf::Vertex(sf::Vector2f(camXTL * zoom, 0), sf::Color::Red));
                        gridVA.append(sf::Vertex(sf::Vector2f(camXBR * zoom, 0), sf::Color::Red));
                    }
                
                    // Render resolution in grid
                    // Left line:
                    gridVA.append(sf::Vertex(sf::Vector2f(0, 0), sf::Color::Green));
                    gridVA.append(sf::Vertex(sf::Vector2f(0, resolution.y * zoom), sf::Color::Green));
                    // Top line:
                    gridVA.append(sf::Vertex(sf::Vector2f(0, 0), sf::Color::Green));
                    gridVA.append(sf::Vertex(sf::Vector2f(resolution.x * zoom, 0), sf::Color::Green));
                    // Right line:
                    gridVA.append(sf::Vertex(sf::Vector2f(resolution.x * zoom, 0), sf::Color::Green));
                    gridVA.append(sf::Vertex(sf::Vector2f(resolution.x * zoom, resolution.y * zoom), sf::Color::Green));
                    // Bottom line:
                    gridVA.append(sf::Vertex(sf::Vector2f(0, resolution.y * zoom), sf::Color::Green));
                    gridVA.append(sf::Vertex(sf::Vector2f(resolution.x * zoom, resolution.y * zoom), sf::Color::Green));
    
                    // Render AABBs, points and edges.
                    // Priorities:
                    // Top, 3rd - Points
                    // Mid, 2nd - Edges
                    // Bot, 1st - AABBs
                    // However, currently selected mode comes last (top)
                    thisEdgeThickness = edgeThickness * (zoom / 32);
                    switch(mode) {
                    case 0:
                        renderEdgeMode(&pointyboxVA, &edgeVec[id][bitmask], sf::Color(255, 255, 0, 48), zoom, thisEdgeThickness);
                        renderPointMode(&pointyboxVA, &pointVec[id][bitmask], sf::Color(255, 0, 0, 48), zoom);
                        renderAABBMode(&pointyboxVA, &aabbVec[id][bitmask], sf::Color(0, 0, 255, 128), zoom);
                        break;
                    case 1:
                        renderAABBMode(&pointyboxVA, &aabbVec[id][bitmask], sf::Color(0, 0, 255, 48), zoom);
                        renderEdgeMode(&pointyboxVA, &edgeVec[id][bitmask], sf::Color(255, 255, 0, 48), zoom, thisEdgeThickness);
                        renderPointMode(&pointyboxVA, &pointVec[id][bitmask], sf::Color(255, 0, 0, 128), zoom);
                        break;
                    case 2:
                        renderAABBMode(&pointyboxVA, &aabbVec[id][bitmask], sf::Color(0, 0, 255, 48), zoom);
                        renderPointMode(&pointyboxVA, &pointVec[id][bitmask], sf::Color(255, 0, 0, 48), zoom);
                        renderEdgeMode(&pointyboxVA, &edgeVec[id][bitmask], sf::Color(255, 255, 0, 128), zoom, thisEdgeThickness);
                        break;
                    }

                    // Render mouse highlight
                    pointyboxVA.append(sf::Vertex(sf::Vector2f(selectedTilePos.x * zoom, selectedTilePos.y * zoom), sf::Color(255, 255, 255, 127)));
                    pointyboxVA.append(sf::Vertex(sf::Vector2f((selectedTilePos.x + 1) * zoom, selectedTilePos.y * zoom), sf::Color(255, 255, 255, 127)));
                    pointyboxVA.append(sf::Vertex(sf::Vector2f((selectedTilePos.x + 1) * zoom, (selectedTilePos.y + 1) * zoom), sf::Color(255, 255, 255, 127)));
                    pointyboxVA.append(sf::Vertex(sf::Vector2f(selectedTilePos.x * zoom, (selectedTilePos.y + 1) * zoom), sf::Color(255, 255, 255, 127)));

                    // Render aabb selection
                    if((mode == 0) && selAABB) {
                        sf::IntRect val(getAABBFromPoints(selectedTilePos, firstPos));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f(val.left * zoom, val.top * zoom), sf::Color(0, 255, 0, 127)));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f((val.left + val.width) * zoom, val.top * zoom), sf::Color(0, 255, 0, 127)));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f((val.left + val.width) * zoom, (val.top + val.height) * zoom), sf::Color(0, 255, 0, 127)));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f(val.left * zoom, (val.top + val.height) * zoom), sf::Color(0, 255, 0, 127)));
                    }
                
                    // Render edge selection
                    if((mode == 2) && selEdge) {
                        sf::IntRect val(getEdgeFromPoints(snapEdge(selectedTilePos, firstPos), firstPos));
                        sf::Color thisColor(0, 255, 0, 127);
                        if((val.left == val.width) && (val.top == val.height))
                            thisColor = sf::Color(255, 0, 0, 127);
                        pointyboxVA.append(sf::Vertex(sf::Vector2f(val.left * zoom - thisEdgeThickness - 1, val.top * zoom - thisEdgeThickness - 1), thisColor));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f(val.width * zoom + thisEdgeThickness, val.top * zoom - thisEdgeThickness - 1), thisColor));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f(val.width * zoom + thisEdgeThickness, val.height * zoom + thisEdgeThickness), thisColor));
                        pointyboxVA.append(sf::Vertex(sf::Vector2f(val.left * zoom - thisEdgeThickness - 1, val.height * zoom + thisEdgeThickness), thisColor));
                    }

                    window.draw(gridVA);
                    window.draw(pointyboxVA);
                    window.draw(infoText);

                    window.display();
                }
            }
            ploader.save(&resolution, &aabbVec, &pointVec, &edgeVec);
        }
        else
            std::cout << "BoxEdit pointybox editor\nUsage: " << argv[0] << " pb_file [guide_file]" << std::endl;
    } catch (const std::exception &exc) {
        std::cerr << "Exception:" << exc.what();
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
