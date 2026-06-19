// nodes/utility/TextInfo.cpp
#include "TextInfo.h"

void TextInfo::defineNode()
{
	addInputPort("input_text", DataType::Text);
	addOutputPort("output_chars", DataType::Numeric);
	addOutputPort("output_lines", DataType::Numeric);
	addOutputPort("output_words", DataType::Numeric);
	addOutputPort("output_bytes", DataType::Numeric);
	addOutputPort("output_summary", DataType::Text);
}

void TextInfo::process()
{
	auto d = getInput("input_text");
	if (!d || d->isNull()) { reportError("输入文本为空"); return; }
	QString text = d->toText();

	int chars = text.length();
	int lines = text.count('\n') + (text.isEmpty() ? 0 : 1);
	int words = 0;
	bool inWord = false;
	for (const QChar& c : text) {
		if (c.isLetterOrNumber()) { if (!inWord) { words++; inWord = true; } }
		else { inWord = false; }
	}
	int bytes = text.toUtf8().size();

	QString summary = QString("%1字符, %2词, %3行, %4字节(UTF-8)")
		.arg(chars).arg(words).arg(lines).arg(bytes);

	setOutput("output_chars", NodeData::createNumeric(chars));
	setOutput("output_lines", NodeData::createNumeric(lines));
	setOutput("output_words", NodeData::createNumeric(words));
	setOutput("output_bytes", NodeData::createNumeric(bytes));
	setOutput("output_summary", NodeData::createText(summary));
}
