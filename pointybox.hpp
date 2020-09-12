#ifndef POINTYBOX_INCLUDED
#define POINTYBOX_INCLUDED

/*
    The pointybox format:
    All it is is just a holder for 3 3D vectors.
    The first vector contains AABB data, while the second and third vectors contain tile corner and side data respectively for lights.
        # (indicates the end of the current vector/start of next vector)
        \n (indicates the end of the first vector level [tile ID], moves to its next index)
        ; (indicates the end of the second vector level [bitmask], moves to its next index)
        , (indicates the end of the current data [x, y, w or h value for a sf::Vector2i or a sf::IntRect], so x moves to y (i.e.) and when pairs or quads are complete, they get recorded.)
    If invalid characters are found (not in above list, numbers or empty spaces), load returns false
*/

#include <fstream>
#include <string.h>
#include <vector>
#include <SFML/Graphics.hpp>

namespace pb {
    // Misc
    struct RangeRect {
        float x1,    // Top left x / min x
              y1,    // Top left y / min y
              x2,    // Bottom right x / max x
              y2;    // Bottom right y / max y
        
        RangeRect(float p_x1, float p_y1, float p_x2, float p_y2);
    };

    struct AALine {
        bool x;     // Is the line X aligned? Like | (true) or like - (false)?
        float a,    // Alignment of fixed axis (value of x if x == true, or y if x == false)
              s,    // Minimum/Smallest (x or y)
              b;    // Maximum/Biggest  (x or y)
        
        AALine(bool p_x, float p_a, float p_s, float p_b);
    };

    // Main containers (special type based)
    typedef std::vector < std::vector < std::vector < RangeRect > > > AABBVector;
    typedef std::vector < std::vector < std::vector < sf::Vector2f > > > PointVector;
    typedef std::vector < std::vector < std::vector < AALine > > > EdgeVector;

    // Raw versions (int based)
    typedef std::vector < std::vector < std::vector < sf::IntRect > > > AABBVectorRaw;
    typedef std::vector < std::vector < std::vector < sf::Vector2i > > > PointVectorRaw;
    typedef std::vector < std::vector < std::vector < sf::IntRect > > > EdgeVectorRaw;

    class PointyboxLoader {
        std::string file;

    public:
        bool load(sf::Vector2u* resolution, AABBVectorRaw* aabbVec, PointVectorRaw* pointVec, EdgeVectorRaw* edgeVec);
        void save(sf::Vector2u* resolution, AABBVectorRaw* aabbVec, PointVectorRaw* pointVec, EdgeVectorRaw* edgeVec); 
        bool parse(sf::Vector2u* resolution, AABBVector* aabbVec, PointVector* pointVec, EdgeVector* edgeVec);
        PointyboxLoader(std::string path);
    };
}

#endif
