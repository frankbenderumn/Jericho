// class SchemaTable {
// private:
//     int id;
//     std::string name;
//     int age;
// public:
//     SchemaTable(int id, const std::string& name, int age) : id(id), name(name), age(age) {}

//     void writeToBinaryFile(const std::string& fileName) {
//         BinaryFile file(fileName, BinaryFile::WRITE);
//         file.writeInt(id);
//         file.writeString(name);
//         file.writeInt(age);
//     }

//     void readFromBinaryFile(const std::string& fileName) {
//         BinaryFile file(fileName, BinaryFile::READ);
//         id = file.readInt();
//         name = file.readString();
//         age = file.readInt();
//     }

//     // Getters and setters omitted for brevity
// };

// class BinaryFile {
// private:
//     std::fstream file;
// public:
//     enum Mode { READ, WRITE };

//     BinaryFile(const std::string& fileName, Mode mode) {
//         std::ios_base::openmode openMode = std::ios_base::binary;
//         if (mode == READ) {
//             openMode |= std::ios_base::in;
//         } else {
//             openMode |= std::ios_base::out;
//         }
//         file.open(fileName, openMode);
//         if (!file.is_open()) {
//             throw std::runtime_error("Failed to open binary file.");
//         }
//     }

//     ~BinaryFile() {
//         if (file.is_open()) {
//             file.close();
//         }
//     }

//     void writeInt(int value) {
//         file.write(reinterpret_cast<const char*>(&value), sizeof(value));
//     }

//     void writeString(const std::string& value) {
//         writeInt(value.size());
//         file.write(value.data(), value.size());
//     }

//     int readInt() {
//         int value = 0;
//         file.read(reinterpret_cast<char*>(&value), sizeof(value));
//         return value;
//     }

//     std::string readString() {
//         int size = readInt();
//         std::string value(size, '\0');
//         file.read(value.data(), size);
//         return value;
//     }

//     // Other methods for handling errors, etc.
// };