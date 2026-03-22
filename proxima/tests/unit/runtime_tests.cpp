void testVectorIndexing() {
    Runtime runtime;

    // Создание вектора
    std::vector<RuntimeValue> vector;
    vector.push_back(RuntimeValue::fromNumber(10));
    vector.push_back(RuntimeValue::fromNumber(20));
    vector.push_back(RuntimeValue::fromNumber(30));

    // Доступ с 1-based индексацией
    RuntimeValue first = runtime.getVectorElement(vector, 1);
    ASSERT_EQ(first.toNumber(), 10.0);

    RuntimeValue second = runtime.getVectorElement(vector, 2);
    ASSERT_EQ(second.toNumber(), 20.0);

    RuntimeValue third = runtime.getVectorElement(vector, 3);
    ASSERT_EQ(third.toNumber(), 30.0);

    // Проверка ошибки при индексе 0
    try {
        runtime.getVectorElement(vector, 0);
        FAIL("Should throw out_of_range for index 0");
    } catch (const std::out_of_range& e) {
        // Ожидаемая ошибка
        PASS();
    }

    // Проверка ошибки при индексе за пределами
    try {
        runtime.getVectorElement(vector, 4);
        FAIL("Should throw out_of_range for index 4");
    } catch (const std::out_of_range& e) {
        // Ожидаемая ошибка
        PASS();
    }
}

void testMatrixIndexing() {
    Runtime runtime;

    // Создание матрицы 3x3
    std::vector<std::vector<RuntimeValue>> matrix(3);
    for (int i = 0; i < 3; i++) {
        matrix[i].resize(3);
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = RuntimeValue::fromNumber((i * 3) + j + 1);
        }
    }

    // Доступ с 1-based индексацией
    // matrix[1,1] = 1, matrix[1,2] = 2, matrix[1,3] = 3
    // matrix[2,1] = 4, matrix[2,2] = 5, matrix[2,3] = 6
    // matrix[3,1] = 7, matrix[3,2] = 8, matrix[3,3] = 9

    RuntimeValue topLeft = runtime.getMatrixElement(matrix, 1, 1);
    ASSERT_EQ(topLeft.toNumber(), 1.0);

    RuntimeValue center = runtime.getMatrixElement(matrix, 2, 2);
    ASSERT_EQ(center.toNumber(), 5.0);

    RuntimeValue bottomRight = runtime.getMatrixElement(matrix, 3, 3);
    ASSERT_EQ(bottomRight.toNumber(), 9.0);

    // Проверка ошибки при индексе 0
    try {
        runtime.getMatrixElement(matrix, 0, 1);
        FAIL("Should throw out_of_range for row index 0");
    } catch (const std::out_of_range& e) {
        PASS();
    }
}
