#include "pointybox.hpp"
//#include <iostream>

pb::RangeRect::RangeRect(float p_x1, float p_y1, float p_x2, float p_y2) :
    x1(p_x1),
    y1(p_y1),
    x2(p_x2),
    y2(p_y2)
{ }

pb::AALine::AALine(bool p_x, float p_a, float p_s, float p_b) :
    x(p_x),
    a(p_a),
    s(p_s),
    b(p_b)
{ }

bool pb::PointyboxLoader::load(sf::Vector2u* resolution, AABBVectorRaw* aabbVec, PointVectorRaw* pointVec, EdgeVectorRaw* edgeVec) {
    std::ifstream fs(file, std::fstream::in | std::fstream::binary);
    if(!fs) {
        fs.close();
        return false;
    }
    std::string str((std::istreambuf_iterator< char >(fs)), std::istreambuf_iterator< char >());
    fs.close();
    unsigned char resLoadState = 0, // 0 = X, 1 = Y, 2 = loaded
                  vec = 0; // 0 = aabb; 1 = point; 2 = edge
    std::string valBuf[4];
    size_t id = 0,
           bitmask = 0,
           n = 0,
           i = 0;
    aabbVec->push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >())); // Fill vectors with initial spaces
    pointVec->push_back(std::vector < std::vector < sf::Vector2i > >(47, std::vector < sf::Vector2i >()));
    edgeVec->push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));
    // Get resolution
    for (std::string resValBuf; i < str.size(); ++i) {
        if (str[i] == '\n') {
            if(resLoadState == 1) {
                unsigned int res = std::stoi(resValBuf);
                if(res < 1)
                    return false;
                resolution->y = res;
                resLoadState = 2;
                resValBuf = "";
                ++i;
                break;
            }
            else {
                unsigned int res = std::stoi(resValBuf);
                if(res < 1)
                    return false;
                resolution->x = res;
                resLoadState = 1;
                resValBuf = "";
            }
        }
        else if ((str[i] >= '0' && str[i] <= '9'))
            resValBuf += str[i];
        else
            return false;
    }
    if(resLoadState != 2)
        return false;
    // Get everything else
    for (; i < str.size(); ++i) {
        if(vec > 2)
            return false;
        if (str[i] == '\n') {
            if(vec == 0)
                aabbVec->push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));
            else if(vec == 1)
                pointVec->push_back(std::vector < std::vector < sf::Vector2i > >(47, std::vector < sf::Vector2i >()));
            else
                edgeVec->push_back(std::vector < std::vector < sf::IntRect > >(47, std::vector < sf::IntRect >()));
            ++id;
            bitmask = 0;
            n = 0;
            for (unsigned char it = 0; it < 4; ++it)
                valBuf[it] = "";
        }
        else if (str[i] == ';') {
            ++bitmask;
            n = 0;
            for (unsigned char it = 0; it < 4; ++it)
                valBuf[it] = "";
        }
        else if (str[i] == ',') {
            ++n;
            unsigned char nMax = (2 + (2 * !(vec == 1)));
            if (n >= nMax) {
                n = 0;
                bool valid = true;
                for (unsigned char check = 0; check < nMax; ++check) {
                    if (valBuf[check].empty()) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    if(vec == 0)
                        aabbVec->at(id)[bitmask].push_back(sf::IntRect(std::stoi(valBuf[0]), std::stoi(valBuf[1]), std::stoi(valBuf[2]), std::stoi(valBuf[3])));
                    else if(vec == 1)
                        pointVec->at(id)[bitmask].push_back(sf::Vector2i(std::stoi(valBuf[0]), std::stoi(valBuf[1])));
                    else
                        edgeVec->at(id)[bitmask].push_back(sf::IntRect(std::stoi(valBuf[0]), std::stoi(valBuf[1]), std::stoi(valBuf[2]), std::stoi(valBuf[3])));
                }
                for (unsigned char it = 0; it < nMax; ++it)
                    valBuf[it] = "";
            }
        }
        else if (str[i] == '#') {
            vec += 1;
            id = 0;
            bitmask = 0;
            n = 0;
            for (unsigned char it = 0; it < 4; ++it)
                valBuf[it] = "";
        }
        else if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-')
            valBuf[n] += str[i];
        else
            return false;
    }
    return true;
}

void pb::PointyboxLoader::save(sf::Vector2u* resolution, AABBVectorRaw* aabbVec, PointVectorRaw* pointVec, EdgeVectorRaw* edgeVec) {
    std::ofstream fs(file, std::fstream::out | std::fstream::binary);
    // Save resolution
    fs << std::to_string(resolution->x) << '\n' << std::to_string(resolution->y) << '\n';
    // Save everything else
    size_t idSize = aabbVec->size();
    for (size_t id = 0; id < idSize; ++id) {
        for (size_t bitmask = 0; bitmask < 47; ++bitmask) {
            size_t itSize = aabbVec->at(id)[bitmask].size();
            for (size_t i = 0; i < itSize; ++i) {
                fs << std::to_string(aabbVec->at(id)[bitmask][i].left) << ',' << std::to_string(aabbVec->at(id)[bitmask][i].top) << ',' << std::to_string(aabbVec->at(id)[bitmask][i].width) << ',' << std::to_string(aabbVec->at(id)[bitmask][i].height) << ',';
            }
            fs << ';';
        }
        if((id + 1) != idSize)
            fs << '\n';
    }
    fs << '#';
    idSize = pointVec->size();
    for (size_t id = 0; id < idSize; ++id) {
        for (size_t bitmask = 0; bitmask < 47; ++bitmask) {
            size_t itSize = pointVec->at(id)[bitmask].size();
            for (size_t i = 0; i < itSize; ++i) {
                fs << std::to_string(pointVec->at(id)[bitmask][i].x) << ',' << std::to_string(pointVec->at(id)[bitmask][i].y) << ',';
            }
            fs << ';';
        }
        if((id + 1) != idSize)
            fs << '\n';
    }
    fs << '#';
    idSize = edgeVec->size();
    for (size_t id = 0; id < idSize; ++id) {
        for (size_t bitmask = 0; bitmask < 47; ++bitmask) {
            size_t itSize = edgeVec->at(id)[bitmask].size();
            for (size_t i = 0; i < itSize; ++i) {
                fs << std::to_string(edgeVec->at(id)[bitmask][i].left) << ',' << std::to_string(edgeVec->at(id)[bitmask][i].top) << ',' << std::to_string(edgeVec->at(id)[bitmask][i].width) << ',' << std::to_string(edgeVec->at(id)[bitmask][i].height) << ',';
            }
            fs << ';';
        }
        if((id + 1) != idSize)
            fs << '\n';
    }
    fs.close();
}

bool pb::PointyboxLoader::parse(sf::Vector2u* resolution, AABBVector* aabbVec, PointVector* pointVec, EdgeVector* edgeVec) {
    AABBVectorRaw aabbVecRaw;
    PointVectorRaw pointVecRaw;
    EdgeVectorRaw edgeVecRaw;
    if(!load(resolution, &aabbVecRaw, &pointVecRaw, &edgeVecRaw))
        return false;
    //std::cout << "Started load. Resolution:" << resolution->x << ";" << resolution->y << std::endl;
    
    for (size_t id = 0; id < aabbVecRaw.size(); ++id) {
        aabbVec->push_back(std::vector < std::vector < RangeRect > >(47, std::vector < RangeRect >()));
        for (size_t bitmask = 0; bitmask < 47; ++bitmask) {
            size_t itSize = aabbVecRaw.at(id)[bitmask].size();
            for (size_t i = 0; i < itSize; ++i) {
                aabbVec->at(id)[bitmask].push_back(RangeRect(
                                                   float(aabbVecRaw.at(id)[bitmask][i].left) / float(resolution->x),
                                                   float(aabbVecRaw.at(id)[bitmask][i].top) / float(resolution->y),
                                                   float(aabbVecRaw.at(id)[bitmask][i].left + aabbVecRaw.at(id)[bitmask][i].width) / float(resolution->x),
                                                   float(aabbVecRaw.at(id)[bitmask][i].top + aabbVecRaw.at(id)[bitmask][i].height) / float(resolution->y)));
                //std::cout << "Load aabb [" << id << "][" << bitmask << "][" << (aabbVec->size() - 1) << "]=" << aabbVec->at(id)[bitmask][aabbVec->size() - 1].x1 << "," << aabbVec->at(id)[bitmask][aabbVec->size() - 1].y1 << "," << aabbVec->at(id)[bitmask][aabbVec->size() - 1].x2 << "," << aabbVec->at(id)[bitmask][aabbVec->size() - 1].y2 << std::endl;
            }
        }
    }
    
    for (size_t id = 0; id < pointVecRaw.size(); ++id) {
        pointVec->push_back(std::vector < std::vector < sf::Vector2f > >(47, std::vector < sf::Vector2f >()));
        for (size_t bitmask = 0; bitmask < 47; ++bitmask) {
            size_t itSize = pointVecRaw.at(id)[bitmask].size();
            for (size_t i = 0; i < itSize; ++i) {
                pointVec->at(id)[bitmask].push_back(sf::Vector2f(
                                                    (0.5f + float(pointVecRaw.at(id)[bitmask][i].x)) / float(resolution->x),
                                                    (0.5f + float(pointVecRaw.at(id)[bitmask][i].y)) / float(resolution->y)
                                                    ));
                //std::cout << "Load point [" << id << "][" << bitmask << "][" << (pointVec->size() - 1) << "]=" << pointVec->at(id)[bitmask][pointVec->size() - 1].x << "," << pointVec->at(id)[bitmask][pointVec->size() - 1].y << std::endl;
            }
        }
    }
    
    for (size_t id = 0; id < edgeVecRaw.size(); ++id) {
        edgeVec->push_back(std::vector < std::vector < AALine > >(47, std::vector < AALine >()));
        for (size_t bitmask = 0; bitmask < 47; ++bitmask) {
            size_t itSize = edgeVecRaw.at(id)[bitmask].size();
            for (size_t i = 0; i < itSize; ++i) {
                bool xAligned;
                float alignedAxisVal,
                      min,
                      max;
                if(edgeVecRaw.at(id)[bitmask][i].left == edgeVecRaw.at(id)[bitmask][i].width) {
                    xAligned = true;
                    alignedAxisVal = float(edgeVecRaw.at(id)[bitmask][i].left) / float(resolution->x);
                }
                else if(edgeVecRaw.at(id)[bitmask][i].top == edgeVecRaw.at(id)[bitmask][i].height) {
                    xAligned = false;
                    alignedAxisVal = float(edgeVecRaw.at(id)[bitmask][i].top) / float(resolution->y);
                }
                else // Two of the values on the same axis must be equal (must be axis aligned)
                    return false;
                    
                if(xAligned) {
                    if(edgeVecRaw.at(id)[bitmask][i].top == edgeVecRaw.at(id)[bitmask][i].height) // Edge mustn't be point
                        return false;
                    min = float(edgeVecRaw.at(id)[bitmask][i].top) / float(resolution->y);
                    max = float(edgeVecRaw.at(id)[bitmask][i].height) / float(resolution->y);
                }
                else {
                    if(edgeVecRaw.at(id)[bitmask][i].left == edgeVecRaw.at(id)[bitmask][i].width) // Edge mustn't be point
                        return false;
                    min = float(edgeVecRaw.at(id)[bitmask][i].left) / float(resolution->x);
                    max = float(edgeVecRaw.at(id)[bitmask][i].width) / float(resolution->x);
                }
                    
                edgeVec->at(id)[bitmask].push_back(AALine(
                                                   xAligned,
                                                   alignedAxisVal,
                                                   min,
                                                   max
                                                  ));
                //std::cout << "Load edge [" << id << "][" << bitmask << "][" << (edgeVec->size() - 1) << "]=" << edgeVec->at(id)[bitmask][edgeVec->size() - 1].x << "," << edgeVec->at(id)[bitmask][edgeVec->size() - 1].a << "," << edgeVec->at(id)[bitmask][edgeVec->size() - 1].s << "," << edgeVec->at(id)[bitmask][edgeVec->size() - 1].b << std::endl;
            }
        }
    }
    return true;
}

pb::PointyboxLoader::PointyboxLoader(std::string path):
    file(path)
{ }
