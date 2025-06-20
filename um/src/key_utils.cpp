#include "key_utils.hpp"
#include <unordered_map>

std::string GetKeyName(int vk) {
    static const std::unordered_map<int, std::string> virtualKeyNames = {
        {0x08, "BACKSPACE"},  {0x09, "TAB"},         {0x10, "SHIFT"},
        {0x11, "CTRL"},       {0x12, "ALT"},         {0x14, "CAPSLOCK"},
        {0x1B, "ESC"},        {0x20, "SPACE"},       {0x25, "LEFT"},
        {0x26, "UP"},         {0x27, "RIGHT"},       {0x28, "DOWN"},
        {0x41, "A"},          {0x42, "B"},           {0x43, "C"},
        {0x44, "D"},          {0x45, "E"},           {0x46, "F"},
        {0x47, "G"},          {0x48, "H"},           {0x49, "I"},
        {0x4A, "J"},          {0x4B, "K"},           {0x4C, "L"},
        {0x4D, "M"},          {0x4E, "N"},           {0x4F, "O"},
        {0x50, "P"},          {0x51, "Q"},           {0x52, "R"},
        {0x53, "S"},          {0x54, "T"},           {0x55, "U"},
        {0x56, "V"},          {0x57, "W"},           {0x58, "X"},
        {0x59, "Y"},          {0x5A, "Z"},           {0x30, "0"},
        {0x31, "1"},          {0x32, "2"},           {0x33, "3"},
        {0x34, "4"},          {0x35, "5"},           {0x36, "6"},
        {0x37, "7"},          {0x38, "8"},           {0x39, "9"},
        {0x70, "F1"},         {0x71, "F2"},          {0x72, "F3"},
        {0x73, "F4"},         {0x74, "F5"},          {0x75, "F6"},
        {0x76, "F7"},         {0x77, "F8"},          {0x78, "F9"},
        {0x79, "F10"},        {0x7A, "F11"},         {0x7B, "F12"},
        {0x01, "Mouse Left"}, {0x02, "Mouse Right"}, {0x04, "Mouse Middle"},
        {0x05, "Mouse X1"},   {0x06, "Mouse X2"}};

    auto it = virtualKeyNames.find(vk);
    if (it != virtualKeyNames.end())
        return it->second;
    return "Unknown";
}
