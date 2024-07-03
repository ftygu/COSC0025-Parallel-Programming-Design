/*
    This file is part of Dirt, the Dartmouth introductory ray tracer.

    Copyright (c) 2017-2019 by Wojciech Jarosz

    Dirt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Dirt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

    ------------------------------------------------------------------------

    This file is based on the WavefrontOBJ mesh class from Nori:

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <dirt/obj.h>
#include <dirt/timer.h>
#include <dirt/progress.h>
#include <unordered_map>
#include <fstream>

namespace
{

/// Tokenize a string into a list by splitting at 'delim'
std::vector<std::string> tokenize(const std::string & string, const std::string & delim = ", ", bool includeEmpty = false)
{
    std::string::size_type lastPos = 0, pos = string.find_first_of(delim, lastPos);
    std::vector<std::string> tokens;

    while (lastPos != std::string::npos)
    {
        if (pos != lastPos || includeEmpty)
            tokens.push_back(string.substr(lastPos, pos - lastPos));
        lastPos = pos;
        if (lastPos != std::string::npos)
        {
            lastPos += 1;
            pos = string.find_first_of(delim, lastPos);
        }
    }

    return tokens;
}

/// Convert a string into an unsigned integer value
unsigned int toUInt(const std::string & str)
{
    char *end_ptr = nullptr;
    unsigned int result = (int) strtoul(str.c_str(), &end_ptr, 10);
    if (*end_ptr != '\0')
        throw DirtException("Could not parse integer value '%s'", str);
    return result;
}

/// Vertex indices used by the OBJ format
struct OBJVertex
{
    uint32_t p = (uint32_t) -1;
    uint32_t n = (uint32_t) -1;
    uint32_t uv = (uint32_t) -1;

    inline OBJVertex()
    {
    }

    inline OBJVertex(const std::string & string)
    {
        std::vector<std::string> tokens = tokenize(string, "/", true);

        if (tokens.size() < 1 || tokens.size() > 3)
            throw DirtException("Invalid vertex data: '%s'", string);

        p = toUInt(tokens[0]);

        if (tokens.size() >= 2 && !tokens[1].empty())
            uv = toUInt(tokens[1]);

        if (tokens.size() >= 3 && !tokens[2].empty())
            n = toUInt(tokens[2]);
    }

    inline bool operator==(const OBJVertex & v) const
    {
        return v.p == p && v.n == n && v.uv == uv;
    }
};

/// Hash function for OBJVertex
struct OBJVertexHash : std::unary_function<OBJVertex, size_t>
{
    std::size_t operator()(const OBJVertex & v) const
    {
        size_t hash = std::hash<uint32_t>()(v.p);
        hash = hash * 37 + std::hash<uint32_t>()(v.uv);
        hash = hash * 37 + std::hash<uint32_t>()(v.n);
        return hash;
    }
};

} // namespace


Mesh loadWavefrontOBJ(const std::string & filename, const Transform & xform)
{
    typedef std::unordered_map<OBJVertex, uint32_t, OBJVertexHash> VertexMap;

    std::ifstream is(filename);
    if (is.fail())
        throw DirtException("Unable to open OBJ file '%s'!", filename);

    message("Loading '%s' ... \n", filename);
    Timer timer;

    std::vector<Vec3f>    positions;
    std::vector<Vec2f>    texcoords;
    std::vector<Vec3f>    normals;
    std::vector<uint32_t>   indices;
    std::vector<OBJVertex>  vertices;
    VertexMap vertexMap;

    // compute line count
    size_t lineCount = 0;
    {
        // new lines will be skipped unless we stop it from happening:    
        is.unsetf(std::ios_base::skipws);

        lineCount = std::count(
            std::istream_iterator<char>(is),
            std::istream_iterator<char>(), 
            '\n');

        is.clear();
        is.seekg(0, is.beg);
    }

    Mesh mesh;
    Box3f bbox;

    {
        Progress progress("Reading OBJ file", lineCount);

        std::string line_str;
        while (std::getline(is, line_str))
        {
            std::istringstream line(line_str);

            std::string prefix;
            line >> prefix;

            if (prefix == "v")
            {
                Vec3f p;
                line >> p.x >> p.y >> p.z;
                p = xform.point(p);
                positions.push_back(p);
            }
            else if (prefix == "vt")
            {
                Vec2f tc;
                line >> tc.x >> tc.y;
                texcoords.push_back(tc);
            }
            else if (prefix == "vn")
            {
                Vec3f n;
                line >> n.x >> n.y >> n.z;
                normals.push_back(normalize(xform.normal(n)));
            }
            else if (prefix == "f")
            {
                std::string v1, v2, v3, v4;
                line >> v1 >> v2 >> v3 >> v4;
                OBJVertex verts[6];
                int nVertices = 3;

                verts[0] = OBJVertex(v1);
                verts[1] = OBJVertex(v2);
                verts[2] = OBJVertex(v3);

                if (!v4.empty())
                {
                    // This is a quad, split into two triangles
                    verts[3] = OBJVertex(v4);
                    verts[4] = verts[0];
                    verts[5] = verts[2];
                    nVertices = 6;
                }

                // Convert to an indexed vertex list
                for (auto i : range(nVertices))
                {
                    const OBJVertex & v = verts[i];
                    VertexMap::const_iterator it = vertexMap.find(v);
                    if (it == vertexMap.end())
                    {
                        vertexMap[v] = (uint32_t) vertices.size();
                        indices.push_back((uint32_t) vertices.size());
                        vertices.push_back(v);
                    }
                    else
                    {
                        indices.push_back(it->second);
                    }
                }
            }
            ++progress;
        }

        mesh.F.resize(indices.size()/3);
        for (auto i : range(int(mesh.F.size())))
            mesh.F[i] = Vec3i(indices[3*i], indices[3*i+1], indices[3*i+2]);

        mesh.V.resize(vertices.size());
        for (auto i : range(int(vertices.size())))
        {
            mesh.V[i] = positions.at(vertices[i].p-1);
            bbox.enclose(mesh.V[i]);
        }

        if (!normals.empty())
        {
            mesh.N.resize(vertices.size());
            for (auto i : range(int(vertices.size())))
                mesh.N[i] = normals.at(vertices[i].n-1);
        }

        if (!texcoords.empty())
        {
            mesh.UV.resize(vertices.size());
            for (auto i : range(int(vertices.size())))
                mesh.UV[i] = texcoords.at(vertices[i].uv-1);
        }

    }

    debug("xform:\n%s\n", xform.m);
    debug("bounding box: min:\n%s;\nmax:\n%s.\n", bbox.pMin, bbox.pMax);

    debug("bounding box: bottom:\n%s;\n", (bbox.pMin+bbox.pMax)/2.f - Vec3f(0, bbox.diagonal()[1]/2.f, 0));

    debug("done. (V=%d, F=%d, took %s and %s)\n",
            mesh.V.size(), mesh.F.size(), timer.elapsedString(),
            memString(mesh.F.size() * sizeof(uint32_t) + sizeof(float) *
                      (mesh.V.size() + mesh.N.size() + mesh.UV.size())));

    return mesh;
}
