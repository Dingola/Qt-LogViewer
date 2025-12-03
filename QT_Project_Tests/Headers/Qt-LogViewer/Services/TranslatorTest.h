#pragma once

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "Qt-LogViewer/Services/Translator.h"

/**
 * @file TranslatorTest.h
 * @brief Test fixture for Translator.
 */
class TranslatorTest: public ::testing::Test
{
    protected:
        TranslatorTest() = default;
        ~TranslatorTest() override = default;

        void SetUp() override;
        void TearDown() override;

        Translator* m_translator = nullptr;
};
