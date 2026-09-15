/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vladyslb <vladyslb@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 09:00:00 by vladyslb          #+#    #+#             */
/*   Updated: 2026/09/15 09:00:00 by vladyslb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

/*
** sys/types.h - системные типы, в частности ssize_t -> это знаковый
** целочисленный тип, то есть он может хранить и положительные, и
** отрицательные значения.
*/
# include <unistd.h>
# include <stdlib.h>
# include <sys/types.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 42
# endif

char	*get_next_line(int fd);

size_t	strlen_at(const char *s, int end);
char	*find_chr(const char *s, int c);
void	*ft_memcpy(void *dest, const void *src, size_t n);
char	*cpy_buffer(const char *buffer, size_t start, size_t len);
char	*merge_previous_and_current(char *previous, const char *current);

#endif
