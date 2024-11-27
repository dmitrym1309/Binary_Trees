#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <filesystem>

struct Train {
    int train_number;
    char from_point[55];
    char to_point[55];
    char date[11];

    void show_data_fields() const {
        std::cout << "Номер поезда: " << train_number
        << ", Из: " << from_point
        << ", В: " << to_point << 
        ", Дата: " << date << std::endl;
    }
};

void convert_text_to_binary(const std::string& text_file, const std::string& binary_file) {
    std::ifstream in_file(text_file);
    std::ofstream out_file(binary_file, std::ios::binary);
    
    Train train;

    while (in_file >> train.train_number) {
        in_file.ignore();
        in_file.getline(train.from_point, sizeof(train.from_point));
        in_file.getline(train.to_point, sizeof(train.to_point));
        in_file.getline(train.date, sizeof(train.date));

        out_file.write(reinterpret_cast<char*>(&train), sizeof(Train));
    }
    
    in_file.close();
    out_file.close();
}

void convert_binary_to_text(const std::string& binary_file, const std::string& text_file) {
    std::ifstream in_file(binary_file, std::ios::binary);
    std::ofstream out_file(text_file);
    
    Train train;
    while (in_file.read(reinterpret_cast<char*>(&train), sizeof(Train))) {
        out_file << train.train_number << std::endl
                 << train.from_point << std::endl
                 << train.to_point << std::endl
                 << train.date << std::endl;
    }
    in_file.close();
    out_file.close();
}

void display_binary_file(const std::string& binary_file) {
    std::ifstream in_file(binary_file, std::ios::binary);
    Train train;

    while (in_file.read(reinterpret_cast<char*>(&train), sizeof(Train))) {
        train.show_data_fields();
    }
    in_file.close();
}

void delete_train(const std::string& binary_file, int train_number) {
    std::fstream file(binary_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        std::cerr << "Ошибка открытия файла." << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::streamsize record_size = sizeof(Train);
    int total_records = file_size / record_size;

    Train current_train;
    Train last_train;
    int last_record_index = total_records - 1;
    bool account_found = false;

    for (int i = 0; i < total_records; ++i) {
        file.read(reinterpret_cast<char*>(&current_train), record_size);

        if (current_train.train_number == train_number) {
            account_found = true;
            std::cout << "Запись с номером поезда " << train_number << " была удалена." << std::endl;
            file.seekg(last_record_index * record_size, std::ios::beg);
            file.read(reinterpret_cast<char*>(&last_train), record_size);
            file.seekp(i * record_size, std::ios::beg);
            file.write(reinterpret_cast<const char*>(&last_train), record_size);
            last_record_index--;
            break;
        }
    }

    file.close();

    if (!account_found) {
        std::cout << "Запись с номером поезда " << train_number << " не найдена." << std::endl;
        return;
    }

    std::filesystem::resize_file(binary_file, (last_record_index + 1) * record_size);
}

Train* find_with_pos(const std::string binary_file, int position) {
    std::ifstream file(binary_file, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка открытия файла." << std::endl;
        return nullptr;
    }
    Train train;
    file.seekg(position * sizeof(Train), std::ios::beg);
    if (file.read(reinterpret_cast<char*>(&train), sizeof(Train))) {
        Train* train_ = new Train();
        *train_ = train;
        return train_;
    } else {
        std::cout << "Запись не найдена." << std::endl;
    }
    file.close();
    return nullptr;
}

Train* find_with_train_number(const std::string& binary_file, int train_number, int& pos) {
    pos = -1;
    std::ifstream file(binary_file, std::ios::binary);
    if (!file) {
        std::cerr << "Ошибка открытия файла." << std::endl;
        return nullptr;
    }

    Train train;
    while (file.read(reinterpret_cast<char*>(&train), sizeof(Train))) {
        if (train.train_number == train_number) {
            Train* train_ = new Train();
            *train_ = train;
            pos = (static_cast<int>(file.tellg()) / sizeof(Train)) - 1;
            return train_;
        }
    }

    file.close();
    std::cout << "Запись не найдена." << std::endl;
    return nullptr;
}

void form_trains_list(const std::string& binary_file, std::string& to_point) {
    std::ifstream file(binary_file, std::ios::binary);
    std::ofstream list_file("accounts.txt");
    if (!file.is_open()) {
        std::cout << "Ошибка открытия файла." << std::endl;
        return;
    }

    Train train;

    while (file.read(reinterpret_cast<char*>(&train), sizeof(Train))) {
        if (train.to_point == to_point) {
            list_file << train.train_number << std::endl;
            list_file << train.from_point << std::endl;
            list_file << train.to_point << std::endl;
            list_file << train.date << std::endl;
        }
    }
    file.close();
    list_file.close();
}

