/*
File format:
row/col
el0.0 el0.1 ... el0.n
el1.0 el1.1 ... el1.n

		...

elm.0 elm.1 ... elm.n
*/
#include <iostream>
#include <string>
#include <fstream>

template <typename item>
item *create_item(item val) {
	item *temp = new item;
	*temp = val;
	return temp;
}

template <typename item>
class KRStruct {
private:
	static const int static_len = 5;

	// Staticheskiy massiv ukazateley na objecti (Item)
	item** static_array[static_len];
	int dyn_len{};
	
	void shiftback(int row, int col){
		if (col < dyn_len && static_array[row][col+1] != NULL) {
			static_array[row][col] = static_array[row][col + 1];
			static_array[row][col + 1] = NULL;
			shiftback(row, col + 1);
		}
	}
	
	void create_new_dyn_array(int new_row) {
		if (new_row < static_len) {
			static_array[new_row] = new item * [dyn_len + 1];
			for (int i{}; i < dyn_len + 1; i++)
				static_array[new_row][i] = NULL;
		}
		else
			std::cout << "Static array is full\n";
	}
	void splat_dyn_array(int old_row, int new_row) {
		for (int old_col{ (dyn_len / 2)+1 }, new_col{}; old_col < dyn_len; old_col++, new_col++) {
			static_array[new_row][new_col] = static_array[old_row][old_col];
			static_array[old_row][old_col] = NULL;
		}
	}
	void merge_dyn_arrays(int old_row, int new_row) {
		for (int old_col{}; old_col < dyn_len; old_col++) {
			if (static_array[old_row][old_col] == NULL && static_array[new_row][0] != NULL) {
				static_array[old_row][old_col] = static_array[new_row][0];
				static_array[new_row][0] = NULL;
				shiftback(new_row, 0);
			}

		}
	}

	int count_elem_in_row(int row) {
		int count{};
		if (static_array[row] != NULL)
			for (int col{}; col < dyn_len; col++)
				if (static_array[row][col] != NULL)
					count++;
		return count;
	}

public:
	KRStruct(int dyn_razmernost) :dyn_len(dyn_razmernost) {
		for (int i{}; i < static_len; i++)
			static_array[i] = NULL;

		static_array[0] = new item*[dyn_len + 1];
		for (int i{}; i < dyn_len+1; i++)
			static_array[0][i] = NULL;
	}
	void push(item *val) {
		for (int i{}; i < static_len; i++) {
			for (int j{}; j < dyn_len; j++) {
				if (static_array[i][j] == NULL) {
					static_array[i][j] = val;
					return;
				}
			}
			if (static_array[i + 1] == NULL) {
				create_new_dyn_array(i + 1);
				splat_dyn_array(i, i + 1);
			}
		}
	}
	item* extract(int row ,int col) {
		if (row < static_len)
			if (col < dyn_len)
				if (static_array[row][col]) {
					item *temp = static_array[row][col];
					static_array[row][col] = NULL;
					shiftback(row, col);
					return temp;
				}
		return NULL;
	}


	int count_all_elements() {
		int counter{};
		for (int row{}; row < static_len; row++)
			if (static_array[row] != NULL)
				for (int col{}; col < dyn_len; col++)
					if (static_array[row][col] != NULL)
						counter++;
		return counter;
	}
	int count_all_rows() {
		int counter{};
		for (int row{}; row < static_len; row++)
			if (static_array[row] != NULL)
				counter++;
		return counter;
	}
	void balance() {
		int amount_elem = count_all_elements();
		int amount_row = count_all_rows();
		
		int elem_per_row = amount_elem / amount_row;
		int additional_elem = amount_elem % amount_row;

		// Sozdadim massiv dlya hranenia elementov
		item **buffer_array = new item*[amount_elem];
		int buffer_index{};
		
		// Vitaskivaem elementi iz dvumernogo massiva v bufferniy
		for (int row{};row<amount_row;row++)
			for (int col{};col<dyn_len;col++)
				if (static_array[row][col] != NULL) {
					buffer_array[buffer_index] = static_array[row][col];
					static_array[row][col] = NULL;
					buffer_index++;
				}

		// Dobavlyaem v dvunerniy massiv osnovnie elementi
		buffer_index = 0;
		for (int row{}; row < amount_row; row++)
			for (int col{}; col < elem_per_row; col++) {
				static_array[row][col] = buffer_array[buffer_index];
				buffer_index++;
			}

		// Dobavlyaem ostalnie
		for (int row{}; row < additional_elem; row++) {
			static_array[row][elem_per_row] = buffer_array[buffer_index];
			buffer_index++;
		}

	}

	bool operator==(const KRStruct &other) {
		for (int row{}; row < static_len; row++)
			if (static_array[row] != NULL)
				for (int col{}; col < dyn_len; col++)
					if (this->static_array[row][col] != NULL && other.static_array[row][col] != NULL)
						if (*this->static_array[row][col] != *other.static_array[row][col])
							return false;
		return true;
	}

	void print_to_file(std::string filename) {
		std::ofstream file(filename);
		file << count_all_rows() << " / " << dyn_len << '\n';

		for (int i{}; i < static_len; i++) {
			if (static_array[i] != NULL)
			{
				for (int j{}; j < dyn_len; j++) {
					if (static_array[i][j] == NULL)
						file << "NULL ";
					else
						file << *static_array[i][j] << ' ';
				}
				file << '\n';
			}
		}


		file.close();
	}

	// Nuzhno chistit massiv, esli chitaem v nepustoi massiv
	void clear() {}

	// Rabotaet only so strokami, potomu chto std::stoi rabotaet tolko so strokami. Mozhno schitat stroki i stolbci vruchnuiu
	void read_from_file(std::string filename) {
		std::ifstream file(filename);

		int file_rows{};
		int file_cols{};

		item temp;
		file >> temp;
		file_rows = std::stoi(temp);
		if (file_rows > static_len) {
		std::cout << "ERROR: Can't contain. Not enough rows\n";
			file.close();
			return;
		}

		file >> temp;	// Skip "/"

		file >> temp;
		file_cols = std::stoi(temp);
		if (file_cols > dyn_len) {
			std::cout << "ERROR: Can't contain. Not enough cols\n";
			file.close();
			return;
		}
		for (int row{ 1 }; row < file_rows; row++) {
			static_array[row] = new item*[dyn_len + 1];
			for (int col{}; col<dyn_len;col++)
				static_array[row][col] = NULL;
		}

		for (int row{};row<file_rows;row++)
			for (int col{}; col < file_cols; col++) {
				file >> temp;
				if (temp == "NULL")
					static_array[row][col] = NULL;
				else {
					item *val = new item;
					*val = temp;
					static_array[row][col] = val;
				}
			}
	}

	// Sortiruem po alphavitu
	void sort() {
		for (int row{}; row < static_len; row++) {
			if (static_array[row]!= NULL)
				for (int iter{}; iter < count_elem_in_row(row) / 2 + 1; iter++) {
					for (int col{ 1 }; col < count_elem_in_row(row)-iter; col++)
						// Sravnivaem pervie bukvi v slovah. Bolshe bukva - dalshe po alphavitu (po ASCII tablice)
						if (tolower(static_array[row][col - 1][0][0]) > tolower(static_array[row][col][0][0])) {
							item *buf = static_array[row][col - 1];
							static_array[row][col - 1] = static_array[row][col];
							static_array[row][col] = buf;
						}
				}
		}
	}

	void show() {
		for (int i{}; i < static_len; i++) {
			if (static_array[i] != NULL)
			{
				for (int j{}; j < dyn_len; j++) {
					if (static_array[i][j] == NULL)
						std::cout << "NULL ";
					else
						std::cout << *static_array[i][j] << ' ';
				}
				std::cout << '\n';
			}
		}
		std::cout << '\n';
	}
};

int main() {
	KRStruct<std::string> test(4);
	
	test.push(create_item<std::string>("Ya"));
	test.push(create_item<std::string>("ebal"));
	test.push(create_item<std::string>("vseh"));
	test.push(create_item<std::string>("prichastnih"));
	test.push(create_item<std::string>("k"));

	test.push(create_item<std::string>("etoy"));
	test.push(create_item<std::string>("huine"));
	test.push(create_item<std::string>("pizdec"));
	test.push(create_item<std::string>("blyat"));

	test.show();
	std::cout << '\n';
	test.balance();
	test.show();
	
	test.sort();
	test.show();

	return 0;
}